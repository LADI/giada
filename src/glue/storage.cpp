/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */

#include "core/model/storage.h"
#include "channel.h"
#include "core/conf.h"
#include "core/engine.h"
#include "core/init.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/sequencer.h"
#include "core/wave.h"
#include "core/waveFactory.h"
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/dialogs/browser/browserLoad.h"
#include "gui/dialogs/browser/browserSave.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/progress.h"
#include "gui/elems/mainWindow/keyboard/column.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/ui.h"
#include "src/core/actions/actionRecorder.h"
#include "storage.h"
#include "utils/fs.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/string.h"
#include <cassert>

extern giada::m::Engine g_engine;
extern giada::v::Ui     g_ui;

namespace giada::c::storage
{
namespace
{
void printLoadError_(int res)
{
	if (res == G_FILE_UNREADABLE)
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_PATCHUNREADABLE));
	else if (res == G_FILE_INVALID)
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_PATCHINVALID));
	else if (res == G_FILE_UNSUPPORTED)
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_PATCHUNSUPPORTED));
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void loadProject(void* data)
{
	v::gdBrowserLoad* browser = static_cast<v::gdBrowserLoad*>(data);

	const std::string projectPath = browser->getSelectedItem();
	const std::string patchPath   = u::fs::join(projectPath, u::fs::stripExt(u::fs::basename(projectPath)) + ".gptc");

	/* Close all sub-windows first, in case there are VST editors visible. VST
	editors must be closed before deleting their plug-in processors. */

	g_ui.closeAllSubwindows();

	auto progress = g_ui.mainWindow->getScopedProgress(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_LOADINGPROJECT));

	m::LoadState state = g_engine.load(projectPath, patchPath, [&progress](float v) { progress.setProgress(v); });
	if (state.patch != G_FILE_OK)
	{
		printLoadError_(state.patch);
		return;
	}

	if (!state.isGood())
		layout::openMissingAssetsWindow(state);

	g_ui.load(g_engine.patch.data);

	browser->do_callback();
}

/* -------------------------------------------------------------------------- */

void saveProject(void* data)
{
	v::gdBrowserSave* browser = static_cast<v::gdBrowserSave*>(data);

	const std::string projectName = u::fs::stripExt(browser->getName());
	const std::string projectPath = u::fs::join(browser->getCurrentPath(), projectName + ".gprj");
	const std::string patchPath   = u::fs::join(projectPath, projectName + ".gptc");

	if (projectName == "")
	{
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_CHOOSEPROJECTNAME));
		return;
	}

	if (u::fs::dirExists(projectPath) &&
	    !v::gdConfirmWin(g_ui.langMapper.get(v::LangMap::COMMON_WARNING),
	        g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_PROJECTEXISTS)))
		return;

	auto progress = g_ui.mainWindow->getScopedProgress(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_SAVINGPROJECT));

	g_ui.store(projectName, g_engine.patch.data);

	if (!g_engine.store(projectName, projectPath, patchPath, [&progress](float v) { progress.setProgress(v); }))
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_SAVINGPROJECTERROR));

	browser->do_callback();
}

/* -------------------------------------------------------------------------- */

void loadSample(void* data)
{
	v::gdBrowserLoad* browser  = static_cast<v::gdBrowserLoad*>(data);
	std::string       fullPath = browser->getSelectedItem();

	if (fullPath.empty())
		return;

	browser->do_callback();

	g_engine.conf.data.samplePath = u::fs::dirname(fullPath);

	c::channel::loadChannel(browser->getChannelId(), fullPath);
}

/* -------------------------------------------------------------------------- */

void saveSample(void* data)
{
	v::gdBrowserSave* browser    = static_cast<v::gdBrowserSave*>(data);
	const std::string name       = browser->getName();
	const std::string folderPath = browser->getCurrentPath();
	const ID          channelId  = browser->getChannelId();

	if (name == "")
	{
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_CHOOSEFILENAME));
		return;
	}

	const std::string filePath = u::fs::join(folderPath, u::fs::stripExt(name) + ".wav");

	if (u::fs::fileExists(filePath) &&
	    !v::gdConfirmWin(g_ui.langMapper.get(v::LangMap::COMMON_WARNING),
	        g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_FILEEXISTS)))
		return;

	if (g_engine.channelManager.saveSample(channelId, filePath))
		g_engine.conf.data.samplePath = u::fs::dirname(filePath);
	else
		v::gdAlert(g_ui.langMapper.get(v::LangMap::MESSAGE_STORAGE_SAVINGFILEERROR));

	browser->do_callback();
}
} // namespace giada::c::storage