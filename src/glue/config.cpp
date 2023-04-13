/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "glue/config.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/engine.h"
#include "core/kernelAudio.h"
#include "core/kernelMidi.h"
#include "core/midiMapper.h"
#include "core/plugins/pluginManager.h"
#include "deps/rtaudio/RtAudio.h"
#include "gui/dialogs/browser/browserDir.h"
#include "gui/dialogs/config.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/config/tabPlugins.h"
#include "gui/ui.h"
#include "utils/fs.h"
#include "utils/vector.h"
#include <cstddef>

extern giada::v::Ui     g_ui;
extern giada::m::Engine g_engine;

namespace giada::c::config
{
namespace
{
AudioDeviceData getAudioDeviceData_(DeviceType type, size_t index, int channelsCount, int channelsStart)
{
	for (const m::KernelAudio::Device& device : g_engine.getAvailableAudioDevices())
		if (device.index == index)
			return AudioDeviceData(type, device, channelsCount, channelsStart);
	return AudioDeviceData();
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

AudioDeviceData::AudioDeviceData(DeviceType type, const m::KernelAudio::Device& device,
    int channelsCount, int channelsStart)
: type(type)
, index(device.index)
, name(device.name)
, channelsMax(type == DeviceType::OUTPUT ? device.maxOutputChannels : device.maxInputChannels)
, sampleRates(device.sampleRates)
, channelsCount(channelsCount)
, channelsStart(channelsStart)
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void AudioData::setOutputDevice(int index)
{
	for (AudioDeviceData& d : outputDevices)
	{
		if (index != d.index)
			continue;
		outputDevice = d;
	}
}

/* -------------------------------------------------------------------------- */

void AudioData::setInputDevice(int index)
{
	for (AudioDeviceData& d : inputDevices)
	{
		if (index == d.index)
		{
			inputDevice = d;
			return;
		}
	}
	inputDevice = {};
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

AudioData getAudioData()
{
	AudioData audioData;

	audioData.apis[RtAudio::Api::RTAUDIO_DUMMY] = "(Dummy)";
	if (g_engine.hasAudioAPI(RtAudio::Api::LINUX_ALSA))
		audioData.apis[RtAudio::Api::LINUX_ALSA] = "ALSA";
	if (g_engine.hasAudioAPI(RtAudio::Api::UNIX_JACK))
		audioData.apis[RtAudio::Api::UNIX_JACK] = "JACK";
	if (g_engine.hasAudioAPI(RtAudio::Api::LINUX_PULSE))
		audioData.apis[RtAudio::Api::LINUX_PULSE] = "PulseAudio";
	if (g_engine.hasAudioAPI(RtAudio::Api::WINDOWS_DS))
		audioData.apis[RtAudio::Api::WINDOWS_DS] = "DirectSound";
	if (g_engine.hasAudioAPI(RtAudio::Api::WINDOWS_ASIO))
		audioData.apis[RtAudio::Api::WINDOWS_ASIO] = "ASIO";
	if (g_engine.hasAudioAPI(RtAudio::Api::WINDOWS_WASAPI))
		audioData.apis[RtAudio::Api::WINDOWS_WASAPI] = "WASAPI";
	if (g_engine.hasAudioAPI(RtAudio::Api::MACOSX_CORE))
		audioData.apis[RtAudio::Api::MACOSX_CORE] = "CoreAudio";

	for (const m::KernelAudio::Device& device : g_engine.getAvailableAudioDevices())
	{
		if (device.maxOutputChannels > 0)
			audioData.outputDevices.push_back(AudioDeviceData(DeviceType::OUTPUT, device, G_MAX_IO_CHANS, 0));
		if (device.maxInputChannels > 0)
			audioData.inputDevices.push_back(AudioDeviceData(DeviceType::INPUT, device, 1, 0));
	}

	const m::model::Layout& layout = g_engine.getLayout();

	audioData.api             = layout.kernelAudio.soundSystem;
	audioData.bufferSize      = layout.kernelAudio.buffersize;
	audioData.sampleRate      = layout.kernelAudio.samplerate;
	audioData.limitOutput     = layout.kernelAudio.limitOutput;
	audioData.recTriggerLevel = layout.kernelAudio.recTriggerLevel;
	audioData.resampleQuality = static_cast<int>(layout.kernelAudio.rsmpQuality);
	audioData.outputDevice    = getAudioDeviceData_(DeviceType::OUTPUT,
        layout.kernelAudio.soundDeviceOut, layout.kernelAudio.channelsOutCount,
        layout.kernelAudio.channelsOutStart);
	audioData.inputDevice     = getAudioDeviceData_(DeviceType::INPUT,
        layout.kernelAudio.soundDeviceIn, layout.kernelAudio.channelsInCount,
        layout.kernelAudio.channelsInStart);

	return audioData;
}

/* -------------------------------------------------------------------------- */

MidiData getMidiData()
{
	MidiData midiData;

	midiData.apis[RtMidi::Api::RTMIDI_DUMMY] = "(Dummy)";
	if (g_engine.hasMidiAPI(RtMidi::Api::LINUX_ALSA))
		midiData.apis[RtMidi::Api::LINUX_ALSA] = "ALSA";
	if (g_engine.hasMidiAPI(RtMidi::Api::UNIX_JACK))
		midiData.apis[RtMidi::Api::UNIX_JACK] = "JACK";
	if (g_engine.hasMidiAPI(RtMidi::Api::WINDOWS_MM))
		midiData.apis[RtMidi::Api::WINDOWS_MM] = "Multimedia MIDI";
	if (g_engine.hasMidiAPI(RtMidi::Api::MACOSX_CORE))
		midiData.apis[RtMidi::Api::MACOSX_CORE] = "OSX Core MIDI";

	midiData.syncModes[G_MIDI_SYNC_NONE]         = "(disabled)";
	midiData.syncModes[G_MIDI_SYNC_CLOCK_MASTER] = "MIDI Clock (master)";
	midiData.syncModes[G_MIDI_SYNC_CLOCK_SLAVE]  = "MIDI Clock (slave)";

	const m::model::Layout& layout = g_engine.getLayout();

	midiData.midiMaps = g_engine.getMidiMapFilesFound();
	midiData.midiMap  = u::vector::indexOf(midiData.midiMaps, g_ui.model.midiMapPath);
	midiData.outPorts = g_engine.getMidiOutPorts();
	midiData.inPorts  = g_engine.getMidiInPorts();
	midiData.api      = layout.kernelMidi.system;
	midiData.syncMode = layout.kernelMidi.sync;
	midiData.outPort  = layout.kernelMidi.portOut;
	midiData.inPort   = layout.kernelMidi.portIn;

	return midiData;
}

/* -------------------------------------------------------------------------- */

PluginData getPluginData()
{
	PluginData pluginData;
	pluginData.numAvailablePlugins = g_engine.getPluginsApi().countAvailablePlugins();
	pluginData.pluginPath          = g_ui.model.pluginPath;
	return pluginData;
}

/* -------------------------------------------------------------------------- */

MiscData getMiscData()
{
	MiscData miscData;
	miscData.logMode      = g_ui.model.logMode;
	miscData.showTooltips = g_ui.model.showTooltips;
	miscData.langMaps     = g_ui.getLangMapFilesFound();
	miscData.langMap      = g_ui.model.langMap;
	miscData.uiScaling    = g_ui.model.uiScaling;
	return miscData;
}
/* -------------------------------------------------------------------------- */

BehaviorsData getBehaviorsData()
{
	const m::model::Layout& layout = g_engine.getLayout();

	BehaviorsData behaviorsData;
	behaviorsData.chansStopOnSeqHalt         = layout.chansStopOnSeqHalt;
	behaviorsData.treatRecsAsLoops           = layout.treatRecsAsLoops;
	behaviorsData.inputMonitorDefaultOn      = layout.inputMonitorDefaultOn;
	behaviorsData.overdubProtectionDefaultOn = layout.overdubProtectionDefaultOn;
	return behaviorsData;
}

/* -------------------------------------------------------------------------- */

void save(const AudioData& data)
{
	m::model::Layout layout             = g_engine.getLayout();
	layout.kernelAudio.soundSystem      = data.api;
	layout.kernelAudio.soundDeviceOut   = data.outputDevice.index;
	layout.kernelAudio.soundDeviceIn    = data.inputDevice.index;
	layout.kernelAudio.channelsOutCount = data.outputDevice.channelsCount;
	layout.kernelAudio.channelsOutStart = data.outputDevice.channelsStart;
	layout.kernelAudio.channelsInCount  = data.inputDevice.channelsCount;
	layout.kernelAudio.channelsInStart  = data.inputDevice.channelsStart;
	layout.kernelAudio.limitOutput      = data.limitOutput;
	layout.kernelAudio.rsmpQuality      = static_cast<m::Resampler::Quality>(data.resampleQuality);
	layout.kernelAudio.buffersize       = data.bufferSize;
	layout.kernelAudio.samplerate       = data.sampleRate;
	layout.kernelAudio.recTriggerLevel  = data.recTriggerLevel;
	g_engine.setLayout(layout);
}

/* -------------------------------------------------------------------------- */

void save(const PluginData& data)
{
	g_ui.model.pluginPath = data.pluginPath;
}

/* -------------------------------------------------------------------------- */

void save(const MidiData& data)
{
	m::model::Layout layout   = g_engine.getLayout();
	layout.kernelMidi.system  = data.api;
	layout.kernelMidi.portOut = data.outPort;
	layout.kernelMidi.portIn  = data.inPort;
	layout.kernelMidi.sync    = data.syncMode;
	g_engine.setLayout(layout);

	g_ui.model.midiMapPath = u::vector::atOr(data.midiMaps, data.midiMap, "");
}

/* -------------------------------------------------------------------------- */

void save(const MiscData& data)
{
	g_ui.model.logMode      = data.logMode;
	g_ui.model.showTooltips = data.showTooltips;
	g_ui.model.langMap      = data.langMap;
	g_ui.model.uiScaling    = std::clamp(data.uiScaling, G_MIN_UI_SCALING, G_MAX_UI_SCALING);
}

/* -------------------------------------------------------------------------- */

void save(const BehaviorsData& data)
{
	m::model::Layout layout           = g_engine.getLayout();
	layout.chansStopOnSeqHalt         = data.chansStopOnSeqHalt;
	layout.treatRecsAsLoops           = data.treatRecsAsLoops;
	layout.inputMonitorDefaultOn      = data.inputMonitorDefaultOn;
	layout.overdubProtectionDefaultOn = data.overdubProtectionDefaultOn;
	g_engine.setLayout(layout);
}

/* -------------------------------------------------------------------------- */

void scanPlugins(std::string dir, const std::function<void(float)>& progress)
{
	g_engine.getPluginsApi().scan(dir, progress);
}

/* -------------------------------------------------------------------------- */

void setPluginPathCb(void* data)
{
	v::gdBrowserDir* browser    = static_cast<v::gdBrowserDir*>(data);
	std::string      pluginPath = g_ui.model.pluginPath;

	if (browser->getCurrentPath() == "")
	{
		v::gdAlert(g_ui.getI18Text(v::LangMap::CONFIG_PLUGINS_INVALIDPATH));
		return;
	}

	if (!pluginPath.empty() && pluginPath.back() != ';')
		pluginPath += ";";
	pluginPath += browser->getCurrentPath();

	g_ui.model.pluginPath = pluginPath;

	browser->do_callback();

	v::gdConfig* configWin = static_cast<v::gdConfig*>(g_ui.getSubwindow(*g_ui.mainWindow.get(), WID_CONFIG));
	configWin->tabPlugins->rebuild();
}
} // namespace giada::c::config
