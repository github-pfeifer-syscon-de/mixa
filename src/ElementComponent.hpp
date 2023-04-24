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

#include <memory>

#include "AlsaElement.hpp"
class ModeComponent;
class ControlComponents;

class ElementComponent {
public:
    ElementComponent(const std::shared_ptr<AlsaElement>& elem, ModeComponent* modeFrame, const std::list<std::shared_ptr<AlsaControl>>& controls, int &n);
    virtual ~ElementComponent();

    void update();
private:

    std::shared_ptr<AlsaElement> m_element;
    std::vector<std::shared_ptr<ControlComponents>> m_controls;
};

