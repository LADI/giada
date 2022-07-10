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

#include "gui/elems/mainWindow/keyboard/midiChannel.h"
#include "core/const.h"
#include "core/graphics.h"
#include "glue/channel.h"
#include "glue/io.h"
#include "glue/layout.h"
#include "glue/recorder.h"
#include "gui/dialogs/warnings.h"
#include "gui/dispatcher.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/menu.h"
#include "gui/elems/basics/statusButton.h"
#include "gui/elems/mainWindow/keyboard/column.h"
#include "gui/elems/mainWindow/keyboard/midiActivity.h"
#include "gui/elems/mainWindow/keyboard/midiChannelButton.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <FL/Fl_Menu_Button.H>
#include <cassert>

extern giada::v::Ui g_ui;

namespace giada::v
{
namespace
{
enum class Menu
{
	EDIT_ACTIONS = 0,
	CLEAR_ACTIONS,
	SETUP_KEYBOARD_INPUT,
	SETUP_MIDI_INPUT,
	SETUP_MIDI_OUTPUT,
	EDIT_ROUTING,
	RENAME_CHANNEL,
	CLONE_CHANNEL,
	DELETE_CHANNEL
};
} // namespace

/* -------------------------------------------------------------------------- */

geMidiChannel::geMidiChannel(int X, int Y, int W, int H, c::channel::Data d)
: geChannel(X, Y, W, H, d)
, m_data(d)
{
	playButton   = new geStatusButton(x(), y(), G_GUI_UNIT, G_GUI_UNIT, channelStop_xpm, channelPlay_xpm);
	arm          = new geButton(playButton->x() + playButton->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, "", armOff_xpm, armOn_xpm);
	mainButton   = new geMidiChannelButton(arm->x() + arm->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, H, m_channel);
	midiActivity = new geMidiActivity(mainButton->x() + mainButton->w() + G_GUI_INNER_MARGIN, y(), 10, h());
	mute         = new geStatusButton(midiActivity->x() + midiActivity->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, muteOff_xpm, muteOn_xpm);
	solo         = new geStatusButton(mute->x() + mute->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, soloOff_xpm, soloOn_xpm);
	fx           = new geStatusButton(solo->x() + solo->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, fxOff_xpm, fxOn_xpm);
	vol          = new geDial(fx->x() + fx->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT);

	end();

	resizable(mainButton);

	playButton->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_LABEL_PLAY));
	arm->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_LABEL_ARM));
	mute->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_LABEL_MUTE));
	solo->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_LABEL_SOLO));
	fx->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_LABEL_FX));
	vol->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_LABEL_VOLUME));

	fx->setStatus(m_channel.plugins.size() > 0);

	playButton->callback(cb_playButton, (void*)this);
	playButton->when(FL_WHEN_CHANGED); // On keypress && on keyrelease

	arm->type(FL_TOGGLE_BUTTON);
	arm->value(m_channel.armed);
	arm->callback(cb_arm, (void*)this);

	fx->callback(cb_openFxWindow, (void*)this);

	mute->type(FL_TOGGLE_BUTTON);
	mute->callback(cb_mute, (void*)this);

	solo->type(FL_TOGGLE_BUTTON);
	solo->callback(cb_solo, (void*)this);

	mainButton->callback(cb_openMenu, (void*)this);

	vol->value(m_channel.volume);
	vol->callback(cb_changeVol, (void*)this);

	size(w(), h()); // Force responsiveness
}

/* -------------------------------------------------------------------------- */

void geMidiChannel::cb_playButton(Fl_Widget* /*w*/, void* p) { ((geMidiChannel*)p)->cb_playButton(); }
void geMidiChannel::cb_openMenu(Fl_Widget* /*w*/, void* p) { ((geMidiChannel*)p)->cb_openMenu(); }

/* -------------------------------------------------------------------------- */

void geMidiChannel::cb_playButton()
{
	g_ui.dispatcher.dispatchTouch(*this, playButton->value());
}

/* -------------------------------------------------------------------------- */

void geMidiChannel::cb_openMenu()
{
	geMenu menu;

	menu.addItem((ID)Menu::EDIT_ACTIONS, g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_EDITACTIONS));
	menu.addItem((ID)Menu::CLEAR_ACTIONS, g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_CLEARACTIONS));
	menu.addItem((ID)Menu::SETUP_KEYBOARD_INPUT, g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_KEYBOARDINPUT));
	menu.addItem((ID)Menu::SETUP_MIDI_INPUT, g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_MIDIINPUT));
	menu.addItem((ID)Menu::SETUP_MIDI_OUTPUT, g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_MIDIOUTPUT));
	menu.addItem((ID)Menu::EDIT_ROUTING, g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_EDITROUTING));
	menu.addItem((ID)Menu::RENAME_CHANNEL, g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_RENAME));
	menu.addItem((ID)Menu::CLONE_CHANNEL, g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_CLONE));
	menu.addItem((ID)Menu::DELETE_CHANNEL, g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_DELETE));

	if (!m_data.hasActions)
		menu.setEnabled((ID)Menu::CLEAR_ACTIONS, false);

	menu.onSelect = [&data = m_data](ID id) {
		switch (static_cast<Menu>(id))
		{
		case Menu::EDIT_ACTIONS:
			c::layout::openMidiActionEditor(data.id);
			break;
		case Menu::CLEAR_ACTIONS:
			c::recorder::clearAllActions(data.id);
			break;
		case Menu::SETUP_KEYBOARD_INPUT:
			c::layout::openKeyGrabberWindow(data.key, [channelId = data.id](int key) {
				return c::io::channel_setKey(channelId, key);
			});
			break;
		case Menu::SETUP_MIDI_INPUT:
			c::layout::openChannelMidiInputWindow(data.id);
			break;
		case Menu::SETUP_MIDI_OUTPUT:
			c::layout::openMidiChannelMidiOutputWindow(data.id);
			break;
		case Menu::EDIT_ROUTING:
			c::layout::openChannelRoutingWindow(data.id);
			break;
		case Menu::CLONE_CHANNEL:
			c::channel::cloneChannel(data.id);
			break;
		case Menu::RENAME_CHANNEL:
			c::layout::openRenameChannelWindow(data);
			break;
		case Menu::DELETE_CHANNEL:
			c::channel::deleteChannel(data.id);
			break;
		}
	};

	menu.popup();
}

/* -------------------------------------------------------------------------- */

void geMidiChannel::resize(int X, int Y, int W, int H)
{
	geChannel::resize(X, Y, W, H);

	arm->hide();
	fx->hide();

	if (w() > BREAK_ARM)
		arm->show();
	if (w() > BREAK_FX)
		fx->show();

	packWidgets();
}
} // namespace giada::v