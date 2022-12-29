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

#include "glue/main.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/engine.h"
#include "core/init.h"
#include "core/kernelAudio.h"
#include "core/kernelMidi.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/recorder.h"
#include "core/sequencer.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "gui/elems/mainWindow/mainIO.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/ui.h"
#include "src/core/actions/actionRecorder.h"
#include "src/core/actions/actions.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/string.h"
#include <FL/Fl.H>
#include <cassert>
#include <cmath>

extern giada::v::Ui     g_ui;
extern giada::m::Engine g_engine;

namespace giada::c::main
{
Timer::Timer(const m::model::Sequencer& c)
: bpm(c.bpm)
, beats(c.beats)
, bars(c.bars)
, quantize(c.quantize)
, isUsingJack(g_engine.getMainEngine().getKernelAudioAPI() == RtAudio::Api::UNIX_JACK)
, isRecordingInput(g_engine.getMainEngine().isRecordingInput())
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

IO::IO(const m::Channel& out, const m::Channel& in, const m::model::Mixer& m)
: masterOutVol(out.volume)
, masterInVol(in.volume)
, masterOutHasPlugins(out.plugins.size() > 0)
, masterInHasPlugins(in.plugins.size() > 0)
, inToOut(m.inToOut)
{
}

/* -------------------------------------------------------------------------- */

Peak IO::getMasterOutPeak()
{
	return g_engine.getMainEngine().getPeakOut();
}

Peak IO::getMasterInPeak()
{
	return g_engine.getMainEngine().getPeakIn();
}

/* -------------------------------------------------------------------------- */

bool IO::isKernelReady()
{
	return g_engine.getMainEngine().isKernelAudioReady();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Timer getTimer()
{
	return Timer(g_engine.model.get().sequencer);
}

/* -------------------------------------------------------------------------- */

IO getIO()
{
	return IO(g_engine.getChannelsEngine().get(m::Mixer::MASTER_OUT_CHANNEL_ID),
	    g_engine.getChannelsEngine().get(m::Mixer::MASTER_IN_CHANNEL_ID),
	    g_engine.model.get().mixer);
}

/* -------------------------------------------------------------------------- */

Sequencer getSequencer()
{
	Sequencer out;

	const m::Mixer::RecordInfo recInfo = g_engine.getMainEngine().getRecordInfo();

	out.isFreeModeInputRec = g_engine.getMainEngine().isRecordingInput() && g_engine.conf.data.inputRecMode == InputRecMode::FREE;
	out.shouldBlink        = g_ui.shouldBlink() && (g_engine.getMainEngine().getSequencerStatus() == SeqStatus::WAITING || out.isFreeModeInputRec);
	out.beats              = g_engine.getMainEngine().getBeats();
	out.bars               = g_engine.getMainEngine().getBars();
	out.currentBeat        = g_engine.getMainEngine().getCurrentBeat();
	out.recPosition        = recInfo.position;
	out.recMaxLength       = recInfo.maxLength;

	return out;
}

/* -------------------------------------------------------------------------- */

Transport getTransport()
{
	Transport transport;
	transport.isRunning         = g_engine.getMainEngine().isSequencerRunning();
	transport.isRecordingAction = g_engine.getMainEngine().isRecordingActions();
	transport.isRecordingInput  = g_engine.getMainEngine().isRecordingInput();
	transport.isMetronomeOn     = g_engine.getMainEngine().isMetronomeOn();
	transport.recTriggerMode    = g_engine.conf.data.recTriggerMode;
	transport.inputRecMode      = g_engine.conf.data.inputRecMode;
	return transport;
}

/* -------------------------------------------------------------------------- */

MainMenu getMainMenu()
{
	MainMenu mainMenu;
	mainMenu.hasAudioData = g_engine.getChannelsEngine().hasChannelsWithAudioData();
	mainMenu.hasActions   = g_engine.getChannelsEngine().hasChannelsWithActions();
	return mainMenu;
}

/* -------------------------------------------------------------------------- */

void setBeats(int beats, int bars)
{
	g_engine.getMainEngine().setBeats(beats, bars);
}

/* -------------------------------------------------------------------------- */

void quantize(int val)
{
	g_engine.getMainEngine().setQuantize(val);
}

/* -------------------------------------------------------------------------- */

void clearAllSamples()
{
	if (!v::gdConfirmWin(g_ui.langMapper.get(v::LangMap::COMMON_WARNING),
	        g_ui.langMapper.get(v::LangMap::MESSAGE_MAIN_FREEALLSAMPLES)))
		return;

	g_ui.closeSubWindow(WID_SAMPLE_EDITOR);
	g_engine.getChannelsEngine().freeAllSampleChannels();
}

/* -------------------------------------------------------------------------- */

void clearAllActions()
{
	if (!v::gdConfirmWin(g_ui.langMapper.get(v::LangMap::COMMON_WARNING),
	        g_ui.langMapper.get(v::LangMap::MESSAGE_MAIN_CLEARALLACTIONS)))
		return;

	g_ui.closeSubWindow(WID_ACTION_EDITOR);
	g_engine.getChannelsEngine().clearAllActions();
}

/* -------------------------------------------------------------------------- */

void setInToOut(bool v)
{
	g_engine.getMainEngine().setInToOut(v);
}

/* -------------------------------------------------------------------------- */

void toggleRecOnSignal()
{
	g_engine.getMainEngine().toggleRecOnSignal();
}

/* -------------------------------------------------------------------------- */

void toggleFreeInputRec()
{
	g_engine.getMainEngine().toggleFreeInputRec();
}

/* -------------------------------------------------------------------------- */

#ifdef G_DEBUG_MODE

void printDebugInfo()
{
	g_engine.model.debug();
}

#endif

/* -------------------------------------------------------------------------- */

void closeProject()
{
	if (!v::gdConfirmWin(g_ui.langMapper.get(v::LangMap::COMMON_WARNING),
	        g_ui.langMapper.get(v::LangMap::MESSAGE_MAIN_CLOSEPROJECT)))
		return;

	g_ui.stopUpdater();
	g_engine.suspend();
	g_engine.reset();
	g_ui.reset();
	g_engine.resume();
	g_ui.startUpdater();
}

/* -------------------------------------------------------------------------- */

void quitGiada()
{
	if (!v::gdConfirmWin(g_ui.langMapper.get(v::LangMap::COMMON_WARNING),
	        g_ui.langMapper.get(v::LangMap::MESSAGE_INIT_QUITGIADA)))
		return;

	m::init::shutdown();
}
} // namespace giada::c::main
