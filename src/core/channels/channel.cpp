/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "core/channels/channel.h"
#include "core/actions/actionRecorder.h"
#include "core/channels/sampleAdvancer.h"
#include "core/conf.h"
#include "core/engine.h"
#include "core/midiMapper.h"
#include "core/mixerHandler.h"
#include "core/model/model.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/recorder.h"
#include <cassert>

extern giada::m::Engine g_engine;

namespace giada::m::channel
{
namespace
{
mcl::AudioBuffer::Pan calcPanning_(float pan)
{
	/* TODO - precompute the AudioBuffer::Pan when pan value changes instead of
	building it on the fly. */

	/* Center pan (0.5f)? Pass-through. */

	if (pan == 0.5f)
		return {1.0f, 1.0f};
	return {1.0f - pan, pan};
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Buffer::Buffer(Frame bufferSize)
: audio(bufferSize, G_MAX_IO_CHANS)
{
}

/* -------------------------------------------------------------------------- */

Data::Data(ChannelType type, ID id, ID columnId, State& s, Buffer& b)
: state(&s)
, buffer(&b)
, id(id)
, type(type)
, columnId(columnId)
, volume(G_DEFAULT_VOL)
, volume_i(G_DEFAULT_VOL)
, pan(G_DEFAULT_PAN)
, armed(false)
, key(0)
, hasActions(false)
, height(G_GUI_UNIT)
, midiLighter(g_engine.midiMapper)
, m_mute(false)
, m_solo(false)
{
	switch (type)
	{
	case ChannelType::SAMPLE:
		samplePlayer.emplace(&(state->resampler.value()));
		sampleAdvancer.emplace();
		sampleReactor.emplace(id, g_engine.sequencer, g_engine.model);
		audioReceiver.emplace();
		sampleActionRecorder.emplace(g_engine.actionRecorder, g_engine.sequencer);
		break;

	case ChannelType::PREVIEW:
		samplePlayer.emplace(&(state->resampler.value()));
		sampleReactor.emplace(id, g_engine.sequencer, g_engine.model);
		break;

	case ChannelType::MIDI:
		midiController.emplace();
		midiSender.emplace(g_engine.kernelMidi);
		midiActionRecorder.emplace(g_engine.actionRecorder, g_engine.sequencer);
#ifdef WITH_VST
		midiReceiver.emplace();
#endif
		break;

	default:
		break;
	}

	initCallbacks();
}

/* -------------------------------------------------------------------------- */

Data::Data(const Patch::Channel& p, State& s, Buffer& b, float samplerateRatio, Wave* wave)
: state(&s)
, buffer(&b)
, id(p.id)
, type(p.type)
, columnId(p.columnId)
, volume(p.volume)
, volume_i(G_DEFAULT_VOL)
, pan(p.pan)
, armed(p.armed)
, key(p.key)
, hasActions(p.hasActions)
, name(p.name)
, height(p.height)
#ifdef WITH_VST
, plugins(g_engine.pluginManager.hydratePlugins(p.pluginIds, g_engine.model)) // TODO move outside, as constructor parameter
#endif
, midiLearner(p)
, midiLighter(g_engine.midiMapper, p)
, m_mute(p.mute)
, m_solo(p.solo)
{
	state->readActions.store(p.readActions);
	state->recStatus.store(p.readActions ? ChannelStatus::PLAY : ChannelStatus::OFF);

	switch (type)
	{
	case ChannelType::SAMPLE:
		samplePlayer.emplace(p, samplerateRatio, &(state->resampler.value()), wave);
		sampleAdvancer.emplace();
		sampleReactor.emplace(id, g_engine.sequencer, g_engine.model);
		audioReceiver.emplace(p);
		sampleActionRecorder.emplace(g_engine.actionRecorder, g_engine.sequencer);
		break;

	case ChannelType::PREVIEW:
		samplePlayer.emplace(p, samplerateRatio, &(state->resampler.value()), nullptr);
		sampleReactor.emplace(id, g_engine.sequencer, g_engine.model);
		break;

	case ChannelType::MIDI:
		midiController.emplace();
		midiSender.emplace(p, g_engine.kernelMidi);
		midiActionRecorder.emplace(g_engine.actionRecorder, g_engine.sequencer);
#ifdef WITH_VST
		midiReceiver.emplace();
#endif
		break;

	default:
		break;
	}

	initCallbacks();
}

/* -------------------------------------------------------------------------- */

Data::Data(const Data& other)
: midiLighter(g_engine.midiMapper)
{
	*this = other;
}

/* -------------------------------------------------------------------------- */

Data& Data::operator=(const Data& other)
{
	if (this == &other)
		return *this;

	state      = other.state;
	buffer     = other.buffer;
	id         = other.id;
	type       = other.type;
	columnId   = other.columnId;
	volume     = other.volume;
	volume_i   = other.volume_i;
	pan        = other.pan;
	m_mute     = other.m_mute;
	m_solo     = other.m_solo;
	armed      = other.armed;
	key        = other.key;
	hasActions = other.hasActions;
	name       = other.name;
	height     = other.height;
#ifdef WITH_VST
	plugins = other.plugins;
#endif

	midiLearner    = other.midiLearner;
	midiLighter    = other.midiLighter;
	samplePlayer   = other.samplePlayer;
	sampleAdvancer = other.sampleAdvancer;
	sampleReactor  = other.sampleReactor;
	audioReceiver  = other.audioReceiver;
	midiController = other.midiController;
#ifdef WITH_VST
	midiReceiver = other.midiReceiver;
#endif
	midiSender           = other.midiSender;
	sampleActionRecorder = other.sampleActionRecorder;
	midiActionRecorder   = other.midiActionRecorder;

	initCallbacks();

	return *this;
}

/* -------------------------------------------------------------------------- */

bool Data::operator==(const Data& other)
{
	return id == other.id;
}

/* -------------------------------------------------------------------------- */

bool Data::isInternal() const
{
	return type == ChannelType::MASTER || type == ChannelType::PREVIEW;
}

bool Data::isMuted() const
{
	/* Internals can't be muted. */
	return !isInternal() && m_mute;
}

bool Data::isSoloed() const
{
	return m_solo;
}

bool Data::canInputRec() const
{
	if (type != ChannelType::SAMPLE)
		return false;

	bool hasWave     = samplePlayer->hasWave();
	bool isProtected = audioReceiver->overdubProtection;
	bool canOverdub  = !hasWave || (hasWave && !isProtected);

	return armed && canOverdub;
}

bool Data::canActionRec() const
{
	return hasWave() && !samplePlayer->isAnyLoopMode();
}

bool Data::hasWave() const
{
	return samplePlayer && samplePlayer->hasWave();
}

bool Data::isPlaying() const
{
	ChannelStatus s = state->playStatus.load();
	return s == ChannelStatus::PLAY || s == ChannelStatus::ENDING;
}

bool Data::isReadingActions() const
{
	ChannelStatus s = state->recStatus.load();
	return s == ChannelStatus::PLAY || s == ChannelStatus::ENDING;
}

/* -------------------------------------------------------------------------- */

void Data::setMute(bool v)
{
	if (m_mute != v)
		midiLighter.sendMute(v);
	m_mute = v;
}

void Data::setSolo(bool v)
{
	if (m_solo != v)
		midiLighter.sendSolo(v);
	m_solo = v;
}

/* -------------------------------------------------------------------------- */

void Data::initCallbacks()
{
	state->playStatus.onChange = [this](ChannelStatus status) {
		midiLighter.sendStatus(status, g_engine.mixer.isChannelAudible(*this));
	};

	if (samplePlayer)
	{
		samplePlayer->onLastFrame = [this]() {
			sampleAdvancer->onLastFrame(*this, g_engine.sequencer.isRunning());
		};
	}
}

/* -------------------------------------------------------------------------- */

void Data::advance(const Sequencer::EventBuffer& events) const
{
	for (const Sequencer::Event& e : events)
	{
		if (midiController)
			midiController->advance(*this, e);
		if (samplePlayer)
			sampleAdvancer->advance(*this, e);
		if (midiSender)
			midiSender->advance(*this, e);
#ifdef WITH_VST
		if (midiReceiver)
			midiReceiver->advance(*this, e);
#endif
	}
}

/* -------------------------------------------------------------------------- */

void Data::react(const EventDispatcher::EventBuffer& events)
{
	for (const EventDispatcher::Event& e : events)
	{
		if (e.channelId > 0 && e.channelId != id)
			continue;

		react(e);
		if (midiController)
			midiController->react(*this, e);
		if (midiSender)
			midiSender->react(*this, e);
		if (samplePlayer)
			samplePlayer->react(e);
		if (midiActionRecorder)
			midiActionRecorder->react(*this, e, g_engine.recorder.canRecordActions());
		if (sampleActionRecorder)
			sampleActionRecorder->react(*this, e, g_engine.conf.data.treatRecsAsLoops,
			    g_engine.sequencer.isRunning(), g_engine.recorder.canRecordActions());
		if (sampleReactor)
			sampleReactor->react(*this, e, g_engine.sequencer, g_engine.conf.data);
#ifdef WITH_VST
		if (midiReceiver)
			midiReceiver->react(*this, e);
#endif
	}
}

/* -------------------------------------------------------------------------- */

void Data::react(const EventDispatcher::Event& e)
{
	switch (e.type)
	{
	case EventDispatcher::EventType::CHANNEL_VOLUME:
		volume = std::get<float>(e.data);
		break;

	case EventDispatcher::EventType::CHANNEL_PAN:
		pan = std::get<float>(e.data);
		break;

	case EventDispatcher::EventType::CHANNEL_MUTE:
		setMute(!isMuted());
		break;

	case EventDispatcher::EventType::CHANNEL_TOGGLE_ARM:
		armed = !armed;
		break;

	case EventDispatcher::EventType::CHANNEL_SOLO:
		setSolo(!isSoloed());
		g_engine.mixerHandler.updateSoloCount();
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void Data::render(mcl::AudioBuffer* out, mcl::AudioBuffer* in, bool audible) const
{
	if (id == Mixer::MASTER_OUT_CHANNEL_ID)
		renderMasterOut(*out);
#ifdef WITH_VST
	else if (id == Mixer::MASTER_IN_CHANNEL_ID)
		renderMasterIn(*in);
#endif
	else
		renderChannel(*out, *in, audible);
}

/* -------------------------------------------------------------------------- */

void Data::renderMasterOut(mcl::AudioBuffer& out) const
{
	buffer->audio.set(out, /*gain=*/1.0f);
#ifdef WITH_VST
	if (plugins.size() > 0)
		g_engine.pluginHost.processStack(buffer->audio, plugins, nullptr);
#endif
	out.set(buffer->audio, volume);
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_VST

void Data::renderMasterIn(mcl::AudioBuffer& in) const
{
	if (plugins.size() > 0)
		g_engine.pluginHost.processStack(in, plugins, nullptr);
}

#endif

/* -------------------------------------------------------------------------- */

void Data::renderChannel(mcl::AudioBuffer& out, mcl::AudioBuffer& in, bool audible) const
{
	buffer->audio.clear();

	if (samplePlayer)
		samplePlayer->render(*this);
	if (audioReceiver)
		audioReceiver->render(*this, in);

		/* If MidiReceiver exists, let it process the plug-in stack, as it can 
	contain plug-ins that take MIDI events (i.e. synths). Otherwise process the
	plug-in stack internally with no MIDI events. */

#ifdef WITH_VST
	if (midiReceiver)
		midiReceiver->render(*this, g_engine.pluginHost);
	else if (plugins.size() > 0)
		g_engine.pluginHost.processStack(buffer->audio, plugins, nullptr);
#endif

	if (audible)
		out.sum(buffer->audio, volume * volume_i, calcPanning_(pan));
}
} // namespace giada::m::channel