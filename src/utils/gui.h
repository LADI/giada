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

#ifndef G_UTILS_GUI_H
#define G_UTILS_GUI_H

#include "core/types.h"
#include <string>

namespace giada::v
{
class gdWindow;
}

namespace giada::u::gui
{
void setFavicon(v::gdWindow* w);

/* removeFltkChars
Strips special chars used by FLTK to split menus into sub-menus. */

std::string removeFltkChars(const std::string& s);

/* getStringWidth
Returns the width in pixels of a string 's'. */

int getStringWidth(const std::string& s);

/* truncate
Adds ellipsis to a string 's' if it longer than 'width' pixels. */

std::string truncate(const std::string& s, Pixel width);

int centerWindowX(int w);
int centerWindowY(int h);
} // namespace giada::u::gui

#endif
