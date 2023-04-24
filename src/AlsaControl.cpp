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

#include <glibmm.h>
#include <math.h>
#include <iostream>

#include "AlsaControl.hpp"
#include "AlsaElement.hpp"

AlsaControl::AlsaControl(AlsaElement *mixerElement, uint32_t flags, uint32_t channel_bits)
: volume_channels{SND_MIXER_SCHN_UNKNOWN, SND_MIXER_SCHN_UNKNOWN}
, pswitch_channels{SND_MIXER_SCHN_UNKNOWN, SND_MIXER_SCHN_UNKNOWN}
, cswitch_channels{SND_MIXER_SCHN_UNKNOWN, SND_MIXER_SCHN_UNKNOWN}
, m_mixerElement{mixerElement}
, m_flags{flags}
, m_channel_bits{channel_bits}
, m_enum_channel_bits{0}
, m_enum_index{0}
{
	snd_mixer_elem_t* elem = mixerElement->getMixerElement();
	for (int i = SND_MIXER_SCHN_FRONT_LEFT; i <= SND_MIXER_SCHN_LAST; ++i)
		if (snd_mixer_selem_get_enum_item(elem, (snd_mixer_selem_channel_id_t)i, &m_enum_index) >= 0)
			m_enum_channel_bits |= 1 << i;
	if (snd_mixer_selem_is_active(elem)) {
		m_flags |= IS_ACTIVE;
	}
}

AlsaControl::~AlsaControl()
{
}

void
AlsaControl::setFlags(uint32_t flags)
{
	m_flags = flags;
}

uint32_t
AlsaControl::getFlags()
{
	return m_flags;
}

void
AlsaControl::replace(std::string& str,
            const std::string& oldStr,
            const std::string& newStr)
{
	std::string::size_type pos = 0u;
	while((pos = str.find(oldStr, pos)) != std::string::npos){
		str.replace(pos, oldStr.length(), newStr);
		pos += newStr.length();
	}
}

bool
AlsaControl::hasVolume()
{
	if (m_flags & IS_ACTIVE &&
		(m_flags & TYPE_PVOLUME ||
		m_flags & TYPE_CVOLUME)) {
		return true;
	}
	return false;
}


std::string
AlsaControl::getName()
{
	if (m_name.empty()) {
		snd_mixer_elem_t* elem = m_mixerElement->getMixerElement();
		uint32_t index = snd_mixer_selem_get_index(elem);
		m_name = snd_mixer_selem_get_name(elem);
		replace(m_name, "IEC958", "S/PDIF");
		if (index > 0) {
			m_name += Glib::ustring::sprintf(" %u", index);
		}
		if (m_flags & IS_MULTICH) {
			m_name += "\n";
			m_name += getMultiChannelName();
		}
	}
	return m_name;
}

std::string
AlsaControl::getShortName()
{
	if (m_shortName.empty()) {
		m_shortName = getName();
		replace(m_shortName, "\n", " ");
	}
	return m_shortName;
}

std::string
AlsaControl::getMultiChannelName()
{
	if (m_flags & IS_MULTICH) {
		uint32_t multiCh = m_flags & MULTICH_MASK;
		switch (multiCh) {
		case 0:
		default:
			return "Front";
			break;
		case 1:
			return "Rear";
			break;
		case 2:
			return "Center";
			break;
		case 3:
			return "Woofer";
			break;
		case 4:
			return "Side";
			break;
		}
	}
	return "";
}

std::string
AlsaControl::getChannelName(snd_mixer_selem_channel_id_t ch)
{
	switch (ch) {
	case SND_MIXER_SCHN_UNKNOWN:
		return "?";
	case SND_MIXER_SCHN_FRONT_LEFT:
		return "frontL (mono)";
	case SND_MIXER_SCHN_FRONT_RIGHT:
		return "frontR";
	case SND_MIXER_SCHN_REAR_LEFT:
		return "rearL";
	case SND_MIXER_SCHN_REAR_RIGHT:
		return "rearR";
	case SND_MIXER_SCHN_FRONT_CENTER:
		return "frontC";
	case SND_MIXER_SCHN_WOOFER:
		return "woofer";
	case SND_MIXER_SCHN_SIDE_LEFT:
		return "sideL";
	case SND_MIXER_SCHN_SIDE_RIGHT:
		return "sideR";
	case SND_MIXER_SCHN_REAR_CENTER:
		return "rearC";
	case SND_MIXER_SCHN_LAST:
		return "last";
	}
	return "-";
}


std::string
AlsaControl::getInfo()
{
	Glib::ustring info;
	info += Glib::ustring::sprintf("%s 0x%04x", m_name, m_flags);
	info += Glib::ustring::sprintf(" %s(%s%s) %s(%s%s)"
		, m_flags & TYPE_PVOLUME ? "pvol" : ""
		, m_flags & HAS_VOLUME_0 ? "0" : ""
		, m_flags & HAS_VOLUME_1 ? " 1" : ""
		, m_flags & TYPE_PSWITCH ? "psw" : ""
		, m_flags & HAS_PSWITCH_0 ? "0" : ""
		, m_flags & HAS_PSWITCH_1 ? " 1" : "");
	info += Glib::ustring::sprintf(" %s %s(%s%s)"
		, m_flags & TYPE_CVOLUME ? "cvol" : ""
		, m_flags & TYPE_CSWITCH ? "csw" : ""
		, m_flags & HAS_CSWITCH_0 ? "0" : ""
		, m_flags & HAS_CSWITCH_1 ? " 1" : "");
	info += Glib::ustring::sprintf(" %s %s %s %s"
		, m_flags & TYPE_ENUM ? "enum" : ""
		, m_flags & IS_MULTICH ? "multi-ch" : "single-ch"
		, m_flags & IS_MULTICH ? getMultiChannelName() : ""
		, m_flags & IS_ACTIVE ? "active" : "inactive");
	info += Glib::ustring::sprintf(" volCh0 %s volCh1 %s "
		, getChannelName(volume_channels[0])
		, getChannelName(volume_channels[1]));

	return info;
}

// for the moment the values are linear and scale from 0..100
//  only left channel is read -> no balance
double
AlsaControl::getVolume()
{
	double value = 0.0;
	snd_mixer_elem_t* elem = m_mixerElement->getMixerElement();
	if (m_flags & TYPE_PVOLUME) {
		long actual,min,max;
		int err = snd_mixer_selem_get_playback_volume(elem, volume_channels[0], &actual);
		if (err >= 0) {
			err = snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
			if (err >= 0) {
				//std::cout << "play " << m_name << " actual " << actual << " min " << min << " max " << max << std::endl;
				actual -= min;
				value = ((double)actual * 100.0) / (double)(max - min);
			}
			else {
				std::cerr << "cannot elem get_playback_range "
					      << getShortName()
					      << " flags " << m_flags
					      << " err " << err
					      << " strerr " << snd_strerror(err) << std::endl;
			}
		}
		else {
			std::cerr << "cannot elem get_playback_vol "
				      << getShortName()
				      << " flags " << m_flags
				      << " err " << err
				      << " strerr " << snd_strerror(err) << std::endl;
		}
	}
	else if (m_flags & TYPE_CVOLUME) {
		long actual,min,max;
		int err = snd_mixer_selem_get_capture_volume(elem, volume_channels[0], &actual);
		if (err >= 0) {
			err = snd_mixer_selem_get_capture_volume_range(elem, &min, &max);
			if (err >= 0) {
				//std::cout << "capt " << m_name << " actual " << actual << " min " << min << " max " << max << std::endl;
				actual -= min;
				value = ((double)actual * 100.0) / (double)(max - min);
			}
			else {
				std::cerr << "cannot elem get_capture_range "
					      << getShortName()
					      << " flags " << m_flags
					      << " err " << err
					      << " strerr " << snd_strerror(err) << std::endl;
			}
		}
		else {
			std::cerr << "cannot elem get_capture_vol "
				      << getShortName()
				      << " flags " << m_flags
				      << " err " << err
				      << " strerr " << snd_strerror(err) << std::endl;
		}
	}
	return value;
}

// for the moment the values are linear and scale from 0..100
//  and target all channels -> no balance
void
AlsaControl::setVolume(double value)
{
	snd_mixer_elem_t* elem = m_mixerElement->getMixerElement();
	if (m_flags & TYPE_PVOLUME) {
		for (uint32_t i = 0; i < ARRAY_SIZE(volume_channels); ++i) {
			long min,max;
			if (volume_channels[i] != SND_MIXER_SCHN_UNKNOWN) {
				int err = snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
				if (err >= 0) {
					long actual = min + (long)(value * (double)(max - min) / 100.0);
					//std::cout << "play " << m_name << " value " << value << " actual " << actual << " min " << min << " max " << max << std::endl;
					err = snd_mixer_selem_set_playback_volume(elem, volume_channels[i], actual);
					if (err < 0) {
						std::cerr << "cannot elem set_playback_vol "
								  << getShortName()
							      << " flags " << m_flags
							      << " err " << err
							      << " strerr " << snd_strerror(err) << std::endl;

					}
				}
				else {
					std::cerr << "cannot elem get_playback_range "
						      << getShortName()
						      << " flags " << m_flags
						      << " err " << err
						      << " strerr " << snd_strerror(err) << std::endl;
				}
			}
		}
	}
	else if (m_flags & TYPE_CVOLUME) {
		for (uint32_t i = 0; i < ARRAY_SIZE(volume_channels); ++i) {
			if (volume_channels[i] != SND_MIXER_SCHN_UNKNOWN) {
				long min,max;
				int err = snd_mixer_selem_get_capture_volume_range(elem, &min, &max);
				if (err >= 0) {
					long actual = min + (long)(value * (double)(max - min) / 100.0);
					//std::cout << "capt " << m_name << " value " << value << " actual " << actual << " min " << min << " max " << max << std::endl;
					err = snd_mixer_selem_set_capture_volume(elem, volume_channels[i], actual);
					if (err < 0) {
						std::cerr << "cannot elem set_capture_vol "
							      << getShortName()
							      << " flags " << m_flags
							      << " err " << err
							      << " strerr " << snd_strerror(err) << std::endl;
					}
				}
				else {
					std::cerr << "cannot elem get_capture_range "
						      << getShortName()
						      << " flags " << m_flags
						      << " err " << err
						      << " strerr " << snd_strerror(err) << std::endl;
				}
			}
		}
	}
}

bool
AlsaControl::hasMute()
{
	if (m_flags & IS_ACTIVE &&
		(m_flags & TYPE_PSWITCH ||
		m_flags & TYPE_CSWITCH)) {
		return true;
	}
	return false;
}

bool
AlsaControl::getMute()
{
	bool value = true;
	snd_mixer_elem_t* elem = m_mixerElement->getMixerElement();
	if (m_flags & TYPE_PSWITCH) {
		int actual;
		int err = snd_mixer_selem_get_playback_switch(elem, pswitch_channels[0], &actual);
		if (err >= 0) {
			//std::cerr << "snd_mixer_selem_get_playback_switch" << m_name << " value " << value << std::endl;
			value = actual == 0;
		}
		else {
			std::cerr << "cannot elem get_playback_switch "
				      << getShortName()
				      << " flags " << m_flags
				      << " err " << err
				      << " strerr " << snd_strerror(err) << std::endl;
		}
	}
	else if (m_flags & TYPE_CSWITCH) {
		int actual;
		int err = snd_mixer_selem_get_capture_switch(elem, cswitch_channels[0], &actual);
		if (err >= 0) {
			//std::cerr << "snd_mixer_selem_get_capture_switch" << m_name << " value " << value << std::endl;
			value = actual == 0;
		}
		else {
			std::cerr << "cannot elem get_capture_switch "
				      << getShortName()
				      << " flags " << m_flags
				      << " err " << err
				      << " strerr " << snd_strerror(err) << std::endl;
		}
	}
	return value;
}

void
AlsaControl::setMute(bool muted)
{
	int actual = muted ? 0 : 1;
	snd_mixer_elem_t* elem = m_mixerElement->getMixerElement();
	if (m_flags & TYPE_PSWITCH) {
		for (uint32_t i = 0; i < ARRAY_SIZE(pswitch_channels); ++i) {
			if (pswitch_channels[i] != SND_MIXER_SCHN_UNKNOWN) {
				int err = snd_mixer_selem_set_playback_switch(elem, pswitch_channels[i], actual);
				if (err < 0) {
					std::cerr << "cannot cntl set_playback_switch "
						      << getShortName()
						      << " flags " << m_flags
						      << " err " << err
						      << " strerr " << snd_strerror(err) << std::endl;
				}
			}
		}
	}
	else if (m_flags & TYPE_CSWITCH) {
		for (uint32_t i = 0; i < ARRAY_SIZE(cswitch_channels); ++i) {
			if (cswitch_channels[i] != SND_MIXER_SCHN_UNKNOWN) {
				int err = snd_mixer_selem_set_capture_switch(elem, cswitch_channels[i], actual);
				if (err < 0) {
					std::cerr << "cannot cntl set_capture_switch "
						      << getShortName()
						      << " flags " << m_flags
						      << " err " << err
						      << " strerr " << snd_strerror(err) << std::endl;
				}
			}
		}
	}
}

AlsaElement*
AlsaControl::getMixerElement()
{
	return m_mixerElement;
}