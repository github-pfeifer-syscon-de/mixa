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
#include <alsa/asoundlib.h>
#include <glibmm.h>

#include "config.h"
#include "AlsaCard.hpp"

AlsaCard::AlsaCard(const std::string& indexstr, const std::string& name, const std::string& device_name)
: m_indexstr{indexstr}
, m_name{name}
, m_device_name{device_name}
{
    #ifdef DEBUG
    std::cout << "AlsaCard::AlsaCard m_indexstr " << m_indexstr
              << " m_name " << m_name
              << " m_device_name " << m_device_name << std::endl;
    #endif
}

AlsaCard::~AlsaCard()
{
}

std::string
AlsaCard::getIndexstr()
{
	return m_indexstr;
}

std::string
AlsaCard::getName()
{
	return m_name;
}

std::string
AlsaCard::getDeviceName()
{
	return m_device_name;
}

std::list<std::shared_ptr<AlsaCard>>
AlsaCard::getCards()
{
	std::list<std::shared_ptr<AlsaCard>> cards;
	snd_ctl_card_info_t *info;
	snd_ctl_card_info_alloca(&info);
	for (int number = -1;;) {
		int err = snd_card_next(&number);
		if (err < 0)
			std::cerr << "cannot enumerate sound cards "
				      << " err " << err
				      << " strerr " << snd_strerror(err) << std::endl;
		if (number < 0)
			break;
#if defined(SND_LIB_VER) && SND_LIB_VER(1, 2, 5) <= SND_LIB_VERSION
		Glib::ustring device_name = Glib::ustring::sprintf("sysdefault:%d", number);
#else
		Glib::ustring device_name = Glib::ustring::sprintf("hw:%d", number);
#endif
		snd_ctl_t *ctl;
		err = snd_ctl_open(&ctl, device_name.c_str(), 0);
		if (err >= 0) {
			err = snd_ctl_card_info(ctl, info);
			snd_ctl_close(ctl);
			if (err >= 0) {
				std::string indexstr(device_name.substr(3));	// will result for alsa < 1.2.5 default:NN
				std::string name(snd_ctl_card_info_get_name(info));
				auto card = std::make_shared<AlsaCard>(indexstr, device_name, name);
				cards.push_back(card);
			}
			else {
				std::cerr << "number no info " << device_name
					      << " err " << err
					      << " strerr " << snd_strerror(err) << std::endl;
			}
		}
		else {
			std::cerr << "number not opened " << device_name
				      << " err " << err
				      << " strerr " << snd_strerror(err) << std::endl;
		}

	}

	return cards;
}

