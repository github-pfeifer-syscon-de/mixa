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

#include <gtkmm.h>
#include <memory>

#include "AlsaControl.hpp"
class ElementComponent;

class ControlComponents  { 
public:
    ControlComponents(const std::shared_ptr<AlsaControl>& cntl, Gtk::Grid* grid, ElementComponent* elem, int col);
    virtual ~ControlComponents();

    void update();
private:
    bool reenableUpdates();
    std::shared_ptr<AlsaControl> m_control;
    Gtk::Scale* m_cntlScale;
    Gtk::CheckButton* m_switch;
    sigc::connection m_timerReenable;
    sigc::connection m_timerValue;
    ElementComponent* m_elem;
};

