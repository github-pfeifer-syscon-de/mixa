/*
 * Copyright (C) 2022 rpf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glibmm.h>
#include <keybinder.h>
#include <string.h>

#include "Keybind.hpp"

#define RAISE_VOLUME_KEY  "XF86AudioRaiseVolume"
#define LOWER_VOLUME_KEY  "XF86AudioLowerVolume"
#define MUTE_KEY          "XF86AudioMute"
#define MIC_MUTE_KEY      "XF86AudioMicMute"
#define PLAY_KEY          "XF86AudioPlay"
#define STOP_KEY          "XF86AudioStop"
#define PREV_KEY          "XF86AudioPrev"
#define NEXT_KEY          "XF86AudioNext"

Keybind::Keybind()
: m_Dispatcher{}
, m_lastKey{KEYTYPE::NONE}
{
	gboolean success;
	keybinder_init(); /* Initialize libkeybinder */
	if (keybinder_supported()) {
		success = (keybinder_bind(LOWER_VOLUME_KEY, Keybind::key_pressed, this) &&
			keybinder_bind(RAISE_VOLUME_KEY, Keybind::key_pressed, this) &&
			keybinder_bind(MUTE_KEY, Keybind::key_pressed, this) &&
			keybinder_bind(MIC_MUTE_KEY, Keybind::key_pressed, this));

		if (!success) {
			g_warning("Could not have grabbed volume control keys. ");
		}
		success = (keybinder_bind(PLAY_KEY, Keybind::key_pressed, this) &&
			keybinder_bind(STOP_KEY, Keybind::key_pressed, this) &&
			keybinder_bind(PREV_KEY, Keybind::key_pressed, this) &&
			keybinder_bind(NEXT_KEY, Keybind::key_pressed, this));

		if (!success) {
			g_warning("Could not have grabbed multimedia control keys.");
		}
	}
	else {
		g_warning("Keybinding not supported.");
	}
}


Keybind::~Keybind()
{
	if (keybinder_supported()) {
		keybinder_unbind(PLAY_KEY, Keybind::key_pressed);
		keybinder_unbind(STOP_KEY, Keybind::key_pressed);
		keybinder_unbind(PREV_KEY, Keybind::key_pressed);
		keybinder_unbind(NEXT_KEY, Keybind::key_pressed);

		keybinder_unbind(LOWER_VOLUME_KEY, Keybind::key_pressed);
		keybinder_unbind(RAISE_VOLUME_KEY, Keybind::key_pressed);
		keybinder_unbind(MUTE_KEY, Keybind::key_pressed);
		keybinder_unbind(MIC_MUTE_KEY, Keybind::key_pressed);
	}
}

void
Keybind::key_pressed(const char *keystring, void *user_data)
{
	//g_warning("%s pressed", keystring);
	Keybind* keybind = (Keybind*) user_data;
	if (keybind) {
		if (strcmp(keystring, RAISE_VOLUME_KEY) == 0) {
			keybind->m_lastKey = KEYTYPE::RAISE_VOLUME;
		}
		else if (strcmp(keystring, LOWER_VOLUME_KEY) == 0) {
			keybind->m_lastKey = KEYTYPE::LOWER_VOLUME;
		}
		else if (strcmp(keystring, MUTE_KEY) == 0) {
			keybind->m_lastKey = KEYTYPE::MUTE;
		}
		else if (strcmp(keystring, MIC_MUTE_KEY) == 0) {
			keybind->m_lastKey = KEYTYPE::MIC_MUTE;
		}
		else if (strcmp(keystring, PLAY_KEY) == 0) {
			keybind->m_lastKey = KEYTYPE::PLAY;
		}
		else if (strcmp(keystring, STOP_KEY) == 0) {
			keybind->m_lastKey = KEYTYPE::STOP;
		}
		else if (strcmp(keystring, PREV_KEY) == 0) {
			keybind->m_lastKey = KEYTYPE::PREV;
		}
		else if (strcmp(keystring, NEXT_KEY) == 0) {
			keybind->m_lastKey = KEYTYPE::NEXT;
		}
		else {
			keybind->m_lastKey = KEYTYPE::NONE;
		}
		keybind->m_Dispatcher.emit();
	}
}

KEYTYPE
Keybind::getKey()
{
	KEYTYPE key = m_lastKey;
	m_lastKey = KEYTYPE::NONE;
	return key;
}

Glib::Dispatcher&
Keybind::getNotification()
{
	return m_Dispatcher;
}
