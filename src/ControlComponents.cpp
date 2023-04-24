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
#include <math.h>

#include "config.h"
#include "AlsaElement.hpp"
#include "ElementComponent.hpp"
#include "ControlComponents.hpp"

ControlComponents::ControlComponents(const std::shared_ptr<AlsaControl>& cntl, 
	Gtk::Grid* grid, ElementComponent* elem, int col)
: m_control{cntl}
, m_cntlScale{nullptr}
, m_switch{nullptr}
, m_timerReenable{}
, m_timerValue{}
, m_elem{elem}
{
	//set_homogeneous(false);
	if (m_control->hasVolume()) {
		m_cntlScale = Gtk::manage(new Gtk::Scale(Gtk::Orientation::ORIENTATION_VERTICAL));
		m_cntlScale->set_range(0, 100);
		m_cntlScale->set_increments(1.0, 10.0);
		m_cntlScale->set_inverted(true);	// start 0 at bottom
		m_cntlScale->set_vexpand(true);
		m_cntlScale->set_draw_value(false);	// prefere scale for value
		for (int i = 0; i <= 100; i += 10) {
			Glib::ustring lbl;
			if (i % 50 == 0) {
				lbl = Glib::ustring::sprintf("%d", i);
			}
			m_cntlScale->add_mark(i, Gtk::PositionType::POS_RIGHT, lbl);
		}
		grid->attach(*m_cntlScale, col, 0);
	}
	else {
		auto strech = Gtk::manage(new Gtk::Box(Gtk::Orientation::ORIENTATION_VERTICAL));
		grid->attach(*strech, col, 0);
	}
	if (m_control->hasMute()) {
		m_switch = Gtk::manage(new Gtk::CheckButton("Mute"));
		//pack_start(*m_switch, false, false, 2);
		grid->attach(*m_switch, col, 1);
	}
	update();
	if (m_cntlScale) {	// install signals later so during update it will not interfere
		m_cntlScale->signal_value_changed().connect([&]() {
			//if (m_timerValue.connected())
			//	m_timerValue.disconnect();
			//m_timerValue = Glib::signal_timeout().connect([&]() {
			//	return false;
			//}, 500u);	// deliver events with some delay ("flood protection")
			// as the scale behaves wired if the same changes get delivered
			//   by alsa->dispatch->here block updates by element
			//if (m_timerReenable.connected())
			//	m_timerReenable.disconnect();
			//m_control->getMixerElement()->setChanging(true);	// block events for this update
            #ifdef DEBUG
			std::cout << "ControlComponents::event " << m_control->getShortName()
					  << " value cntl "  << m_cntlScale->get_value()
					  << " value alsa "  << m_control->getVolume()
					  << std::endl;
            #endif
			//double diff = std::abs(m_control->getVolume() - m_cntlScale->get_value());
			//if (diff > 1.0) {	// prevent event flood
				m_control->setVolume(m_cntlScale->get_value());
			//}
			//m_timerReenable = Glib::signal_timeout().connect(		// use timer to reenable updating
			//		sigc::mem_fun(*this, &ControlComponents::reenableUpdates), 100u);
		});
	}
	if (m_switch) {
		m_switch->signal_toggled().connect([&]() {
			//std::cout << "ControlComponents::event " << m_control->getShortName()
			//		  << (m_control->getMute() ? " muted" : " not muted")
			//	      << std::endl;
			bool muted = m_switch->get_active();
			if (m_control->getMute() != muted) {
				m_control->setMute(muted);
			}
			if (m_cntlScale) {
				m_cntlScale->set_sensitive(!muted);
			}
		});
	}

	auto lbl = Gtk::manage(new Gtk::Label(cntl->getName()));
	lbl->property_valign().set_value(Gtk::Align::ALIGN_START);	// align text to top
	grid->attach(*lbl, col, 2);

}


ControlComponents::~ControlComponents()
{
}

bool
ControlComponents::reenableUpdates()
{
	//m_elem->setChanging(false);
	return false;	// disconnect timer (do only once)
}

void
ControlComponents::update()
{
	// alsa->ui
	if (m_switch) {
		//std::cout << "ControlComponents::update " << m_control->getShortName()
		//		  << (m_control->getMute() ? " muted" : " not muted")
		//		  << std::endl;
		bool muted = m_control->getMute();
		if (muted != m_switch->get_active()) {
			m_switch->set_active(muted);
		}
		if (m_cntlScale) {
			m_cntlScale->set_sensitive(!muted);
		}
	}
	if (m_cntlScale) {
		std::cout << "ControlComponents::update " << m_control->getShortName()
			      << " value cntl "  << m_cntlScale->get_value()
				  << " value alsa "  << m_control->getVolume()
				  << std::endl;
		//std::cout << "ElementFrame::update volume " << m_control->getVolume() << std::endl;
		//double diff = std::abs(m_control->getVolume() - m_cntlScale->get_value());
		//if (diff > 1.0) {	// prevent event flood
			m_cntlScale->set_value(m_control->getVolume());
		//}
	}
}
