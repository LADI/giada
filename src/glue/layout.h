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

#ifndef G_GLUE_LAYOUT_H
#define G_GLUE_LAYOUT_H

#include "core/types.h"
#include <functional>
#include <string>

namespace giada::m
{
struct LoadState;
}

namespace giada::v
{
class gdWindow;
}

namespace giada::c::channel
{
struct Data;
}

namespace giada::c::layout
{
void openBrowserForProjectLoad();
void openBrowserForProjectSave();
void openBrowserForSampleLoad(ID channelId);
void openBrowserForSampleSave(ID channelId);
void openAboutWindow();
void openKeyGrabberWindow(int key, std::function<bool(int)>);
void openBpmWindow(std::string bpmValue);
void openBeatsWindow(int beats, int bars);
void openConfigWindow();
void openMasterMidiInputWindow();
void openChannelMidiInputWindow(ID channelId);
void openChannelRoutingWindow(const c::channel::Data&);
void openSampleChannelMidiOutputWindow(ID channelId);
void openMidiChannelMidiOutputWindow(ID channelId);
void openSampleActionEditor(ID channelId);
void openMidiActionEditor(ID channelId);
void openSampleEditor(ID channelId);
void openRenameChannelWindow(const c::channel::Data&);
void openMissingAssetsWindow(const m::LoadState&);
#ifdef WITH_VST
void openBrowserForPlugins(v::gdWindow& parent);
void openChannelPluginListWindow(ID channelId);
void openMasterInPluginListWindow();
void openMasterOutPluginListWindow();
void openPluginChooser(ID channelId);
#endif
} // namespace giada::c::layout

#endif
