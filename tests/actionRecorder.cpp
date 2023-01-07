#include "src/core/actions/actionRecorder.h"
#include "src/core/actions/action.h"
#include "src/core/actions/actions.h"
#include "src/core/channels/channelFactory.h"
#include "src/core/conf.h"
#include "src/core/const.h"
#include "src/core/model/model.h"
#include "src/core/types.h"
#include <catch2/catch.hpp>

TEST_CASE("ActionRecorder")
{
	using namespace giada;
	using namespace giada::m;

	const ID channel1 = 1;
	const ID channel2 = 2;

	Conf::Data     conf;
	model::Model   model;
	ChannelFactory channelFactory(conf, model);

	model.registerThread(Thread::MAIN, /*realtime=*/false);
	model.reset();
	model.get().channels = {
	    channelFactory.create(channel1, ChannelType::SAMPLE, 0, 0, 1024),
	    channelFactory.create(channel2, ChannelType::SAMPLE, 0, 0, 1024)};
	model.swap(model::SwapType::NONE);

	ActionRecorder ar(model);

	REQUIRE(ar.hasActions(channel1) == false);

	SECTION("Test record")
	{
		const Frame     f1 = 10;
		const Frame     f2 = 70;
		const MidiEvent e1 = MidiEvent::makeFrom3Bytes(MidiEvent::CHANNEL_NOTE_ON, 0x00, 0x00, 0);
		const MidiEvent e2 = MidiEvent::makeFrom3Bytes(MidiEvent::CHANNEL_NOTE_OFF, 0x00, 0x00, 0);

		const Action a1 = ar.rec(channel1, f1, e1);
		const Action a2 = ar.rec(channel1, f2, e2);

		REQUIRE(ar.hasActions(channel1) == true);
		REQUIRE(a1.frame == f1);
		REQUIRE(a2.frame == f2);
		REQUIRE(a1.prevId == 0);
		REQUIRE(a1.nextId == 0);
		REQUIRE(a2.prevId == 0);
		REQUIRE(a2.nextId == 0);

		SECTION("Test clear actions by channel")
		{
			const Frame     f1 = 100;
			const Frame     f2 = 200;
			const MidiEvent e1 = MidiEvent::makeFrom3Bytes(MidiEvent::CHANNEL_NOTE_ON, 0x00, 0x00, 0);
			const MidiEvent e2 = MidiEvent::makeFrom3Bytes(MidiEvent::CHANNEL_NOTE_OFF, 0x00, 0x00, 0);

			ar.rec(channel2, f1, e1);
			ar.rec(channel2, f2, e2);

			ar.clearChannel(channel1);

			REQUIRE(ar.hasActions(channel1) == false);
			REQUIRE(ar.hasActions(channel2) == true);
		}

		SECTION("Test clear actions by type")
		{
			ar.clearActions(channel1, MidiEvent::CHANNEL_NOTE_ON);
			ar.clearActions(channel1, MidiEvent::CHANNEL_NOTE_OFF);

			REQUIRE(ar.hasActions(channel1) == false);
		}

		SECTION("Test clear all")
		{
			ar.clearAllActions();
			REQUIRE(ar.hasActions(channel1) == false);
		}
	}
}
