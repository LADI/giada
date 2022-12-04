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

#ifndef G_EVENT_DISPATCHER_H
#define G_EVENT_DISPATCHER_H

#include "core/actions/action.h"
#include "core/const.h"
#include "core/ringBuffer.h"
#include "core/types.h"
#include "core/worker.h"
#include "deps/concurrentqueue/concurrentqueue.h"
#include <any>
#include <atomic>
#include <functional>
#include <thread>

/* giada::m::EventDispatcher
Takes events from the a lock-free queue filled by c::events and turns them into 
actual changes in the data model. The EventDispatcher runs in a separate worker 
thread. */

namespace giada::m
{
class EventDispatcher
{
public:
	enum class EventType
	{
		KEY_PRESS,
		KEY_RELEASE,
		KEY_KILL,
		SEQUENCER_START,
		SEQUENCER_STOP,
		SEQUENCER_REWIND,
		SEQUENCER_BPM,
		SEQUENCER_GO_TO_BEAT,
		RECORDER_PREPARE_ACTION_REC,
		RECORDER_PREPARE_INPUT_REC,
		RECORDER_STOP_ACTION_REC,
		RECORDER_STOP_INPUT_REC,
#ifdef WITH_AUDIO_JACK
		SEQUENCER_START_JACK,
		SEQUENCER_STOP_JACK,
		SEQUENCER_REWIND_JACK,
		SEQUENCER_BPM_JACK,
#endif
		MIDI,
		MIDI_DISPATCHER_LEARN,
		MIDI_DISPATCHER_PROCESS,
		MIXER_SIGNAL_CALLBACK,
		MIXER_END_OF_REC_CALLBACK,
		CHANNEL_TOGGLE_READ_ACTIONS,
		CHANNEL_KILL_READ_ACTIONS,
		CHANNEL_TOGGLE_ARM,
		CHANNEL_MUTE,
		CHANNEL_SOLO,
		CHANNEL_VOLUME,
		CHANNEL_PITCH,
		CHANNEL_PAN
	};

	struct Event
	{
		EventType type;
		Frame     delta     = 0;
		ID        channelId = 0;
		std::any  data      = {};
	};

	/* EventBuffer
	Alias for a RingBuffer containing events to be sent to engine. */

	using EventBuffer = RingBuffer<Event, G_MAX_DISPATCHER_EVENTS>;

	EventDispatcher();

	/* start
	Starts the internal worker on a separate thread. Call this on startup. */

	void start();

	/* pumpEvent
	Inserts a new event in the event queue. Returns false if the queue is full. */

	bool pumpEvent(const Event&);

	/* on[...]
	Callbacks fired when something happens in the Event Dispatcher. */

	std::function<void(const MidiEvent& e)> onMidiLearn;
	std::function<void(const MidiEvent& e)> onMidiProcess;
	std::function<void(const EventBuffer&)> onProcessChannels;
	std::function<void(const EventBuffer&)> onProcessSequencer;
	std::function<void()>                   onMixerSignalCallback;
	std::function<void()>                   onMixerEndOfRecCallback;

private:
	void processFunctions();
	void process();

	/* m_worker
	A separate thread responsible for the event processing. */

	Worker m_worker;

	/* m_eventBuffer
	Buffer of events sent to channels for event parsing. This is filled with 
	Events coming from the event queue.*/

	EventBuffer m_eventBuffer;

	/* m_eventQueue
	Collects events coming from the UI or MIDI devices. */

	moodycamel::ConcurrentQueue<Event> m_eventQueue;
};
} // namespace giada::m

#endif
