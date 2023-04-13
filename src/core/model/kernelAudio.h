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

#ifndef G_MODEL_KERNEL_AUDIO_H
#define G_MODEL_KERNEL_AUDIO_H

#include "core/resampler.h"
#include "core/types.h"
#include "deps/rtaudio/RtAudio.h"

namespace giada::m::model
{
struct KernelAudio
{
	bool               ready            = false;
	RtAudio::Api       soundSystem      = G_DEFAULT_SOUNDSYS;
	int                soundDeviceOut   = G_DEFAULT_SOUNDDEV_OUT;
	int                soundDeviceIn    = G_DEFAULT_SOUNDDEV_IN;
	int                channelsOutCount = G_MAX_IO_CHANS;
	int                channelsOutStart = 0;
	int                channelsInCount  = 1;
	int                channelsInStart  = 0;
	int                samplerate       = G_DEFAULT_SAMPLERATE;
	unsigned int       buffersize       = G_DEFAULT_BUFSIZE;
	bool               limitOutput      = false;
	Resampler::Quality rsmpQuality      = Resampler::Quality::LINEAR;
	float              recTriggerLevel  = 0.0f;
};
} // namespace giada::m::model

#endif
