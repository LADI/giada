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

#ifndef G_ENGINE_H
#define G_ENGINE_H

#include "core/IOEngine.h"
#include "core/actionEditorEngine.h"
#include "core/actions/actionRecorder.h"
#include "core/actions/actions.h"
#include "core/channels/channelFactory.h"
#include "core/channels/channelManager.h"
#include "core/channelsEngine.h"
#include "core/conf.h"
#include "core/eventDispatcher.h"
#include "core/init.h"
#include "core/jackTransport.h"
#include "core/kernelAudio.h"
#include "core/kernelMidi.h"
#include "core/mainEngine.h"
#include "core/midiDispatcher.h"
#include "core/midiMapper.h"
#include "core/midiSynchronizer.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/pluginsEngine.h"
#include "core/recorder.h"
#include "core/sampleEditorEngine.h"
#include "core/sequencer.h"
#include "core/storageEngine.h"
#include "core/waveFactory.h"
#ifdef WITH_AUDIO_JACK
#include "core/jackSynchronizer.h"
#endif

namespace giada::m
{
class Engine final
{
public:
	/* Engine()
    Prepares all sub-components by constructing them and setting up the required
    callback for inter-component communication. It doesn't start the engine yet. */

	Engine();

	bool                             isAudioReady() const;
	bool                             hasAudioAPI(RtAudio::Api) const;
	bool                             hasMidiAPI(RtMidi::Api) const;
	RtAudio::Api                     getAudioAPI() const;
	std::vector<KernelAudio::Device> getAudioDevices() const;
	std::vector<std::string>         getMidiOutPorts() const;
	std::vector<std::string>         getMidiInPorts() const;
	int                              getSampleRate() const;
	int                              getBufferSize() const;

	/* getPatch
	Returns a read-only reference to the current loaded Patch. */

	const Patch::Data& getPatch() const;

	/* getConf
	Returns a reference to the current loaded Conf. */

	Conf::Data& getConf();

	/* updateMixerModel
	Updates some values in model::Mixer data struct needed by m::Mixer for the
	audio rendering. Call this whenever the audio configuration changes. */

	void updateMixerModel();

	/* init
    Initializes all sub-components. If KernelAudio fails to start, the process
    interrupts and Giada is put in an invalid state. */

	void init();

	/* reset
    Resets all sub-components to the initial state. Useful when Giada needs to
    be brought back to the startup state. */

	void reset();

	/* shutdown
    Closes the current audio device. */

	void shutdown();

	/* suspend, resume
	Toggles Mixer's rendering operation. */

	void suspend();
	void resume();

#ifdef G_DEBUG_MODE
	void debug();
#endif

	/* setOnModelSwapCb
	Sets callback to fire when the model gets swapped. */

	void setOnModelSwapCb(std::function<void(model::SwapType)>);

	/* getMainEngine
	Return a reference to MainEngine, responsible for the central component	API. */

	MainEngine& getMainEngine();

	/* getChannelsEngine
	Return a reference to ChannelsEngine, responsible for the channels API. */

	ChannelsEngine& getChannelsEngine();

	/* getPluginsEngine
	Return a reference to ChannelsEngine, responsible for the plug-ins API. */

	PluginsEngine& getPluginsEngine();

	/* getSampleEditorEngine
	Return a reference to SampleEditorEngine, responsible for the Sample Editor 
	API. */

	SampleEditorEngine& getSampleEditorEngine();

	/* getActionEditorEngine
	Return a reference to ActionEditorEngine, responsible for the Action Editor 
	API. */

	ActionEditorEngine& getActionEditorEngine();

	/* getIOEngine
	Return a reference to IOEngine, responsible for the MIDI I/O API. */

	IOEngine& getIOEngine();

	/* getIOEngine
	Return a reference to StorageEngine, responsible for persistence. */

	StorageEngine& getStorageEngine();

	/* get[... component ...]
	Returns a reference to an internal. TODO - these methods will be removed with
	new Channel rendering architecture */

	KernelMidi&     getKernelMidi();
	ActionRecorder& getActionRecorder();
	PluginHost&     getPluginHost();

	MidiMapper<KernelMidi> midiMapper;

private:
	int audioCallback(KernelAudio::CallbackInfo) const;

	void storeConfig();
	void loadConfig();

	Conf             m_conf;
	Patch            m_patch;
	model::Model     m_model;
	KernelAudio      m_kernelAudio;
	KernelMidi       m_kernelMidi;
	PluginHost       m_pluginHost;
	JackTransport    m_jackTransport;
	MidiSynchronizer m_midiSynchronizer;
	Sequencer        m_sequencer;
	Mixer            m_mixer;
	ChannelManager   m_channelManager;
	ActionRecorder   m_actionRecorder;
	Recorder         m_recorder;
	PluginManager    m_pluginManager;
	EventDispatcher  m_eventDispatcher;
	MidiDispatcher   m_midiDispatcher;
#ifdef WITH_AUDIO_JACK
	JackSynchronizer m_jackSynchronizer;
#endif

	MainEngine         m_mainEngine;
	ChannelsEngine     m_channelsEngine;
	PluginsEngine      m_pluginsEngine;
	SampleEditorEngine m_sampleEditorEngine;
	ActionEditorEngine m_actionEditorEngine;
	IOEngine           m_ioEngine;
	StorageEngine      m_storageEngine;
};
} // namespace giada::m

#endif
