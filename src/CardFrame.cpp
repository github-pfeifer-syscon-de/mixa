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
#include <memory>

#include "config.h"
#include "ControlComponents.hpp"
#include "ModeComponent.hpp"
#include "CardFrame.hpp"

CardFrame::CardFrame(const std::shared_ptr<AlsaCard>& card)
: Gtk::Box{Gtk::Orientation::ORIENTATION_HORIZONTAL, 2}
, m_card{card}
, m_mixer{}
{
}

CardFrame::~CardFrame()
{
}

void
CardFrame::on_show()
{
	Gtk::Box::on_show();

	if (get_children().empty()) {
		selectMixer();
	}
}

void
CardFrame::on_hide()
{
	Gtk::Box::on_hide();

	if (m_mixer) {
		m_mixer->removeFromAllMixers();	// required to exit idle loop+remove static reference
		m_mixer.reset();
	}
	//std::cout << "CardFrame::on_hide cnt " <<  m_mixer.use_count() << std::endl;
}

void
CardFrame::selectMixer()
{
    #ifdef DEBUG
    std::cout << "selectMixer " << m_card->getName() << std::endl;
    #endif
   	if (!m_mixer) {
		m_mixer = std::make_shared<AlsaMixer>(m_card);
		AlsaMixer::addToAllMixers(m_mixer);
	}
	m_mixer->attach(m_card->getName());
	//std::cout << "Page " << cardName <<  " has " << cardPage->get_children().size() << ( m_mixer->isUnplugged() ? "unplugged" : "plugged" ) << std::endl;
    if (!m_mixer->isUnplugged()) {
		//std::cout << "Create playCapt Notebook " << cardName <<  " has " << get_children().size() << std::endl;
		auto playCaptNotebook = Gtk::manage(new Gtk::Notebook());
		playCaptNotebook->set_tab_pos(Gtk::PositionType::POS_RIGHT);
		pack_start(*playCaptNotebook, true, true, 0);

		addModePages(playCaptNotebook);
		show_all_children();
    }
    else {
        std::cerr << "Mixer not plugged" << std::endl;
    }
}


void
CardFrame::addModePages(Gtk::Notebook* playCaptNotebook)
{
	ModeComponent* playFrame = nullptr;
	ModeComponent* captFrame = nullptr;
	auto elements = m_mixer->getElements();
	for (auto elem : elements) {
		auto playCntls = elem->getPlayControls();
		if (!playCntls.empty()) {
			if (playFrame == nullptr) {
				playFrame = Gtk::manage(new ModeComponent());
				playCaptNotebook->append_page(*playFrame, "Play", false);
			}
			playFrame->addControls(elem, playCntls);
		}
		auto captCntls = elem->getCaptureControls();
		if (!captCntls.empty()) {
			if (captFrame == nullptr) {
				captFrame = Gtk::manage(new ModeComponent());
				playCaptNotebook->append_page(*captFrame, "Capture", false);
			}
			captFrame->addControls(elem, captCntls);
		}
	}
}

std::shared_ptr<AlsaCard>
CardFrame::getCard()
{
    return m_card;
}

std::shared_ptr<AlsaMixer>
CardFrame::getMixer()
{
	return m_mixer;
}