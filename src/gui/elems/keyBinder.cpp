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

#include "gui/elems/keyBinder.h"
#include "core/const.h"
#include "glue/layout.h"
#include "gui/dialogs/keyGrabber.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/button.h"
#include "utils/gui.h"

namespace giada::v
{
geKeyBinder::geKeyBinder(const std::string& l, int& keyRef)
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
{
	m_labelBox = new geBox(l.c_str());
	m_keyBox   = new geBox(u::gui::keyToString(keyRef).c_str());
	m_bindBtn  = new geButton("Bind");
	m_clearBtn = new geButton("Clear");

	add(m_labelBox);
	add(m_keyBox, 100);
	add(m_bindBtn, 50);
	add(m_clearBtn, 50);
	end();

	m_labelBox->box(G_CUSTOM_BORDER_BOX);
	m_keyBox->box(G_CUSTOM_BORDER_BOX);

	m_bindBtn->onClick = [&keyRef, this]() {
		c::layout::openKeyGrabberWindow(keyRef, [&keyRef, this](int newKey) {
			keyRef = newKey;
			m_keyBox->copy_label(u::gui::keyToString(keyRef).c_str());
			return true;
		});
	};

	m_clearBtn->onClick = [&keyRef, this]() {
		keyRef = 0;
		m_keyBox->copy_label(u::gui::keyToString(keyRef).c_str());
	};
}

} // namespace giada::v