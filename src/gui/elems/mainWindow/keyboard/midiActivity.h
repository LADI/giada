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

#ifndef GE_MIDI_ACTIVITY_H
#define GE_MIDI_ACTIVITY_H

#include "core/types.h"
#include "gui/elems/basics/group.h"
#include "gui/elems/basics/pack.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>

namespace giada::v
{
class geMidiActivity : public Fl_Group
{
public:
	class geLed : public Fl_Button
	{
	public:
		geLed(int x, int y, int w, int h);

		void draw() override;
		void lit();

	private:
		int m_decay;
	};

	geMidiActivity(int x, int y, int w, int h);

	void resize(int x, int y, int w, int h) override;

	geLed* out;
	geLed* in;

private:
	Fl_Group* m_top;
	Fl_Group* m_bot;
};
} // namespace giada::v

#endif
