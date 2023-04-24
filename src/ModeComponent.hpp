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

#include <vector>
#include <memory>
#include <gtkmm.h>

#include "AlsaControl.hpp"
#include "AlsaMixer.hpp"
class ElementComponent;

class ModeComponent : public Gtk::Grid {
public:
    ModeComponent();
    virtual ~ModeComponent();

    void addControls(const std::shared_ptr<AlsaElement> elem, const std::list<std::shared_ptr<AlsaControl>>& controls);
private:

    std::vector<std::shared_ptr<ElementComponent>> m_elements;
    int m_n;
};

