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
#include <memory.h>
#include <glibmm.h>

#include "config.h"
#include "AlsaCard.hpp"
#include "AlsaEvents.hpp"
#include "AlsaMixer.hpp"

std::list<std::shared_ptr<AlsaMixer>> AlsaMixer::s_allMixers{};

AlsaMixer::AlsaMixer(std::shared_ptr<AlsaCard>& card)
: m_mixer{nullptr}
, m_mixer_device_name{}
, m_unplugged{true}
, m_elements{}
, m_alsaEvents{nullptr}
{
    #ifdef DEBUG
	std::cout << "Mixer::Mixer start " << card->getName() << std::endl;
    #endif
	int err;

	err = snd_mixer_open(&m_mixer, 0);
	if (err < 0) {
		std::cerr << "cannot mixer open " << err << " msg " << snd_strerror(err) << std::endl;
		return;
	}
	struct snd_mixer_selem_regopt selem_regopt = {
		.ver = 1,
		.abstract = SND_MIXER_SABSTRACT_NONE,
		.device = card->getName().c_str(), // alsamixer uses e.g. default,sysdefault0
		.playback_pcm = nullptr,
		.capture_pcm = nullptr
	};
	m_mixer_device_name = card->getName();
	err = snd_mixer_selem_register(m_mixer, &selem_regopt, NULL);
	if (err < 0) {
		std::cerr << "cannot mixer selem_register " << err << " msg " << snd_strerror(err) << std::endl;
		return;
	}

	snd_mixer_set_callback(m_mixer, AlsaMixer::mixer_callback);

	m_unplugged = false;
    #ifdef DEBUG
    std::cout << "Mixer::Mixer end" << std::endl;
    #endif
}


AlsaMixer::~AlsaMixer()
{
    #ifdef DEBUG
    std::cout << "AlsaMixer::~AlsaMixer" << std::endl;
    #endif
	if (m_alsaEvents) {
		delete m_alsaEvents;
		m_alsaEvents = nullptr;
	}
	close_hctl();
	if (m_mixer) {
		snd_mixer_close(m_mixer);
	}
}

void
AlsaMixer::close_hctl()
{
	if (!m_mixer_device_name.empty()) {
        #ifdef DEBUG
        std::cout << "AlsaMixer::close_hctl" << std::endl;
        #endif
        snd_mixer_detach(m_mixer, m_mixer_device_name.c_str());
		m_mixer_device_name = "";
	}
	m_elements.clear();
}

bool
AlsaMixer::isUnplugged()
{
	return m_unplugged;
}

void
AlsaMixer::attach(const std::string& mixer_device_name)
{
	close_hctl();
	// if this is included load will report -22 invalid argument
    #ifdef DEBUG
    std::cout << "AlsaMixer::attach " << mixer_device_name << std::endl;
    #endif
	int err = snd_mixer_attach(m_mixer, mixer_device_name.c_str());
	if (err < 0) {
		std::cerr << "Cannot mixer attach device " << mixer_device_name
			      << " err " << err
			      << " strerr " << snd_strerror(err) << std::endl;
		return;
	}
	m_mixer_device_name = mixer_device_name;

    #ifdef DEBUG
    std::cout << "AlsaMixer::attach " << mixer_device_name << " load" << std::endl;
    #endif
	err = snd_mixer_load(m_mixer);
	if (err < 0) {
		std::cerr << "cannot load mixer controls "
			      << " err " << err
				  << " strerr " << snd_strerror(err) << std::endl;
		return;
	}
	m_unplugged = false;
	m_alsaEvents = new AlsaThreadEvents(m_mixer);		// multi threaded (rare involuntary changes)
	//m_alsaEvents = new AlsaIdleEvents(m_mixer);		// single threaded (uses full process)
}

int
AlsaMixer::mixer_callback(snd_mixer_t *mixer, unsigned int mask, snd_mixer_elem_t *elem)
{
	//std::cout << "mixer_callback mask 0x" << std::hex << mask << std::dec << std::endl;
	if (mask & SND_CTL_EVENT_MASK_ADD) {
		snd_mixer_elem_set_callback(elem, AlsaMixer::elem_callback);
		for (auto alsaMixer : s_allMixers) {
			if (alsaMixer->m_mixer == mixer) {
				alsaMixer->createElement(elem);
				break;
			}
		}
		//AlsaMixer::s_controls_changed = true;
	}
	return 0;
}

int
AlsaMixer::elem_callback(snd_mixer_elem_t *elem, unsigned int mask)
{
	//std::cout << "elem_callback mask 0x" << std::hex << mask << std::dec << std::endl;
	if (mask == SND_CTL_EVENT_MASK_REMOVE) {
		//AlsaMixer::s_controls_changed = true;
	}
	else {
		if (mask & SND_CTL_EVENT_MASK_VALUE) {
			//AlsaMixer::s_control_values_changed = true;
			for (auto alsaMix : s_allMixers) {
				for (auto alsaElem : alsaMix->getElements()) {
					if (elem == alsaElem->getMixerElement()) {
						alsaElem->update();
						break;
					}
				}
			}
		}
		if (mask & SND_CTL_EVENT_MASK_INFO) {
			//AlsaMixer::s_controls_changed = true;
		}
	}

	return 0;
}

std::list<std::shared_ptr<AlsaElement>>
AlsaMixer::getElements()
{
    #ifdef DEBUG
    std::cout << "AlsaMixer::getElements start " << m_elements.size() << std::endl;
    #endif
	if (m_elements.empty()) {
        #ifdef DEBUG
        std::cout << "AlsaMixer::getElements creating" << std::endl;
        #endif
		snd_mixer_elem_t *elem = snd_mixer_first_elem(m_mixer);
		while (elem != NULL) {
			createElement(elem);
			elem = snd_mixer_elem_next(elem);
		}
	}
    #ifdef DEBUG
    std::cout << "AlsaMixer::getElements end " << m_elements.size() << std::endl;
    #endif
	return m_elements;
}

void
AlsaMixer::createElement(snd_mixer_elem_t *elem)
{
	auto alsaElem = std::make_shared<AlsaElement>(elem);
	m_elements.push_back(alsaElem);
}

void
AlsaMixer::addToAllMixers(const std::shared_ptr<AlsaMixer>& mixer)
{
	s_allMixers.push_back(mixer);
}

// as we created a sticky reference with s_allMixers
//   we require this to destruct properly
void
AlsaMixer::removeFromAllMixers()
{
    #ifdef DEBUG
    std::cout << "AlsaMixer::removeFromAllMixers" << std::endl;
    #endif
	if (m_alsaEvents) {
		m_alsaEvents->destroy();
	}
	for (auto mix : s_allMixers) {
		if (mix.get() == this) {
			//std::cout << "found in close" << std::endl;
			s_allMixers.remove(mix);
			break;
		}
	}
}

