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


#include "ControlComponents.hpp"
#include "AlsaControl.hpp"
#include "AlsaElement.hpp"
#include "ModeComponent.hpp"
#include "ElementComponent.hpp"

ElementComponent::ElementComponent(const std::shared_ptr<AlsaElement>& elem,
	ModeComponent* modeFrame, const std::list<std::shared_ptr<AlsaControl>>& controls, int& n)
: m_element{elem}
, m_controls{}
{
	for (auto cntl : controls) {
		auto controlComponents = std::make_shared<ControlComponents>(cntl, modeFrame, this, n++);
		m_controls.push_back(controlComponents);
	}
	// get updates from elsewhere
	elem->getNotifications()
				.connect(sigc::mem_fun(*this, &ElementComponent::update));

}

ElementComponent::~ElementComponent()
{
}

void
ElementComponent::update()
{
	for (auto cntl : m_controls) {
		cntl->update();
	}
}

