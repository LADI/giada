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

#ifndef GE_TAB_BEHAVIORS_H
#define GE_TAB_BEHAVIORS_H

#include "core/conf.h"
#include "deps/geompp/src/rect.hpp"
#include "gui/elems/basics/check.h"
#include <FL/Fl_Group.H>

namespace giada::v
{
class geTabBehaviors : public Fl_Group
{
public:
	geTabBehaviors(geompp::Rect<int>, m::Conf::Data&);

	void save();

private:
	geCheck* m_chansStopOnSeqHalt;
	geCheck* m_treatRecsAsLoops;
	geCheck* m_inputMonitorDefaultOn;
	geCheck* m_overdubProtectionDefaultOn;

	m::Conf::Data& m_conf;
};
} // namespace giada::v

#endif
