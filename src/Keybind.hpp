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

#pragma once


enum class KEYTYPE {
    NONE
    ,RAISE_VOLUME
    ,LOWER_VOLUME
    ,MUTE
    ,MIC_MUTE
    ,PLAY
    ,STOP
    ,PREV
    ,NEXT
  };


class Keybind {
public:
    Keybind();
    Keybind(const Keybind& orig) = delete;
    virtual ~Keybind();

    Glib::Dispatcher& getNotification();
    KEYTYPE getKey();
private:
    static void key_pressed(const char *keystring, void *user_data);

    Glib::Dispatcher m_Dispatcher;  // used for key notification
    KEYTYPE m_lastKey;
};
