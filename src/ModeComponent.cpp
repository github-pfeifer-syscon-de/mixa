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

#include <iostream>

#include "ModeComponent.hpp"
#include "ElementComponent.hpp"

ModeComponent::ModeComponent()
: Gtk::Grid{}
, m_elements{}
, m_n{0}
{
	set_column_spacing(3u);
	set_row_homogeneous(false);
}

ModeComponent::~ModeComponent()
{
}

void
ModeComponent::addControls(const std::shared_ptr<AlsaElement> elem, const std::list<std::shared_ptr<AlsaControl>>& controls)
{
	auto elemComp = std::make_shared<ElementComponent>(elem, this, controls, m_n);
	m_elements.push_back(elemComp);
}