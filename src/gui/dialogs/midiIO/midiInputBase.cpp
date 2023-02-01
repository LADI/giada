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

#include "gui/dialogs/midiIO/midiInputBase.h"
#include "core/conf.h"
#include "glue/io.h"
#include "utils/gui.h"

namespace giada::v
{
gdMidiInputBase::gdMidiInputBase(const char* title, m::Conf& c)
: gdWindow(u::gui::getCenterWinBounds(c.midiInputBounds), title)
, m_conf(c)
{
}

/* -------------------------------------------------------------------------- */

gdMidiInputBase::~gdMidiInputBase()
{
	c::io::stopMidiLearn();

	m_conf.midiInputBounds = getBounds();
}
} // namespace giada::v