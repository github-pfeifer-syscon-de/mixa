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
#include <glibmm.h>

#include "AlsaElement.hpp"
#include "AlsaControl.hpp"

static const snd_mixer_selem_channel_id_t supported_channels[] = {
	SND_MIXER_SCHN_FRONT_LEFT,
	SND_MIXER_SCHN_FRONT_RIGHT,
	SND_MIXER_SCHN_REAR_LEFT,
	SND_MIXER_SCHN_REAR_RIGHT,
	SND_MIXER_SCHN_FRONT_CENTER,
	SND_MIXER_SCHN_WOOFER,
	SND_MIXER_SCHN_SIDE_LEFT,
	SND_MIXER_SCHN_SIDE_RIGHT,
};

static const snd_mixer_selem_channel_id_t control_channels[][2] = {
	{ SND_MIXER_SCHN_FRONT_LEFT, SND_MIXER_SCHN_FRONT_RIGHT },
	{ SND_MIXER_SCHN_REAR_LEFT, SND_MIXER_SCHN_REAR_RIGHT },
	{ SND_MIXER_SCHN_FRONT_CENTER, SND_MIXER_SCHN_UNKNOWN },
	{ SND_MIXER_SCHN_WOOFER, SND_MIXER_SCHN_UNKNOWN },
	{ SND_MIXER_SCHN_SIDE_LEFT, SND_MIXER_SCHN_SIDE_RIGHT },
};

AlsaElement::AlsaElement(snd_mixer_elem_t *elem)
: m_selem_id{nullptr}
, m_elem{elem}
, m_playControls{}
, m_captControls{}
, m_notifySignal{}
{
	int err = snd_mixer_selem_id_malloc(&m_selem_id);
	if (err < 0) {
		std::cerr << "out of memory";
	}
}

AlsaElement::~AlsaElement()
{
	if (m_selem_id)
		snd_mixer_selem_id_free(m_selem_id);

}

std::list<std::shared_ptr<AlsaControl>>
AlsaElement::getPlayControls()
{
	if (m_playControls.empty()) {
		m_playControls = create_controls_for_elem(VIEW_MODE_PLAYBACK);
	}
	return m_playControls;
}

std::list<std::shared_ptr<AlsaControl>>
AlsaElement::getCaptureControls()
{
	if (m_captControls.empty()) {
		m_captControls = create_controls_for_elem(VIEW_MODE_CAPTURE);
	}
	return m_captControls;
}

std::list<std::shared_ptr<AlsaControl>>
AlsaElement::create_controls_for_elem(view_mode viewMode)
{
	snd_mixer_elem_t* elem = m_elem;
	std::list<std::shared_ptr<AlsaControl>> controls;
	unsigned int i;
	bool has_channel[LAST_SUPPORTED_CHANNEL + 1];
	bool has_ch0, has_ch1;
	std::shared_ptr<AlsaControl> front_control;

	if (snd_mixer_elem_get_type(elem) != SND_MIXER_ELEM_SIMPLE)
		return controls;
	if (snd_mixer_selem_is_enumerated(elem)) {
		if ((viewMode == VIEW_MODE_PLAYBACK && snd_mixer_selem_is_enum_capture(elem)) ||
		    (viewMode == VIEW_MODE_CAPTURE && !snd_mixer_selem_is_enum_capture(elem)))
			return controls;
		auto control = std::make_shared<AlsaControl>(this, TYPE_ENUM, 0);
		controls.push_back(control);
		return controls;
	}
	bool has_pvol = snd_mixer_selem_has_playback_volume(elem);
	bool has_psw = snd_mixer_selem_has_playback_switch(elem);
	bool has_cvol = snd_mixer_selem_has_capture_volume(elem);
	bool has_csw = snd_mixer_selem_has_capture_switch(elem);
	bool merged_cswitch = viewMode == VIEW_MODE_ALL && has_merged_cswitch(elem);
	if (viewMode != VIEW_MODE_CAPTURE && (has_pvol || has_psw)) {
		if ((!has_pvol || snd_mixer_selem_has_playback_volume_joined(elem)) &&
		    (!has_psw || snd_mixer_selem_has_playback_switch_joined(elem))) {
			auto control = std::make_shared<AlsaControl>(this, 0, 0);
			if (has_pvol) {
				control->setFlags(control->getFlags() | TYPE_PVOLUME | HAS_VOLUME_0);
				control->volume_channels[0] = SND_MIXER_SCHN_FRONT_LEFT;
			}
			if (has_psw) {
				control->setFlags(control->getFlags() | TYPE_PSWITCH | HAS_PSWITCH_0);
				control->pswitch_channels[0] = SND_MIXER_SCHN_FRONT_LEFT;
			}
			if (merged_cswitch) {
				control->setFlags(control->getFlags() | TYPE_CSWITCH);
				if (snd_mixer_selem_has_capture_switch_joined(elem)) {
					control->setFlags(control->getFlags() | HAS_CSWITCH_0);
					control->cswitch_channels[0] = SND_MIXER_SCHN_FRONT_LEFT;
				} else {
					if (snd_mixer_selem_has_capture_channel(elem, control_channels[0][0])) {
						control->setFlags(control->getFlags() | HAS_CSWITCH_0);
						control->cswitch_channels[0] = control_channels[0][0];
					}
					if (control_channels[0][1] != SND_MIXER_SCHN_UNKNOWN &&
					    snd_mixer_selem_has_capture_channel(elem, control_channels[0][1])) {
						control->setFlags(control->getFlags() | HAS_CSWITCH_1);
						control->cswitch_channels[1] = control_channels[0][1];
					}
				}
				if ((control->getFlags() & (HAS_CSWITCH_0 | HAS_CSWITCH_1)) == HAS_CSWITCH_1) {
					control->setFlags(control->getFlags()  ^ (HAS_CSWITCH_0 | HAS_CSWITCH_1));
					control->cswitch_channels[0] = control->cswitch_channels[1];
				}
			}
			controls.push_back(control);
		} else {
			for (i = 0; i < ARRAY_SIZE(supported_channels); ++i)
				has_channel[supported_channels[i]] =
					snd_mixer_selem_has_playback_channel(elem, supported_channels[i]);
			for (i = 0; i < ARRAY_SIZE(control_channels); ++i) {
				has_ch0 = has_channel[control_channels[i][0]];
				has_ch1 = control_channels[i][1] != SND_MIXER_SCHN_UNKNOWN &&
					has_channel[control_channels[i][1]];
				if (!has_ch0 && !has_ch1)
					continue;
				auto control = std::make_shared<AlsaControl>(this, 0, 0);
				if (has_pvol) {
					control->setFlags(control->getFlags() | TYPE_PVOLUME);
					if (snd_mixer_selem_has_playback_volume_joined(elem)) {
						control->setFlags(control->getFlags() | HAS_VOLUME_0);
						control->volume_channels[0] = SND_MIXER_SCHN_FRONT_LEFT;
					} else {
						if (has_ch0) {
							control->setFlags(control->getFlags() | HAS_VOLUME_0);
							control->volume_channels[0] = control_channels[i][0];
						}
						if (has_ch1) {
							control->setFlags(control->getFlags() | HAS_VOLUME_1);
							control->volume_channels[1] = control_channels[i][1];
						}
					}
				}
				if (has_psw) {
					control->setFlags(control->getFlags() | TYPE_PSWITCH);
					if (snd_mixer_selem_has_playback_switch_joined(elem)) {
						control->setFlags(control->getFlags() | HAS_PSWITCH_0);
						control->pswitch_channels[0] = SND_MIXER_SCHN_FRONT_LEFT;
					} else {
						if (has_ch0) {
							control->setFlags(control->getFlags() | HAS_PSWITCH_0);
							control->pswitch_channels[0] = control_channels[i][0];
						}
						if (has_ch1) {
							control->setFlags(control->getFlags() | HAS_PSWITCH_1);
							control->pswitch_channels[1] = control_channels[i][1];
						}
					}
				}
				if (merged_cswitch) {
					control->setFlags(control->getFlags() | TYPE_CSWITCH);
					if (snd_mixer_selem_has_capture_switch_joined(elem)) {
						control->setFlags(control->getFlags() | HAS_CSWITCH_0);
						control->cswitch_channels[0] = SND_MIXER_SCHN_FRONT_LEFT;
					} else {
						if (snd_mixer_selem_has_capture_channel(elem, control_channels[i][0])) {
							control->setFlags(control->getFlags() | HAS_CSWITCH_0);
							control->cswitch_channels[0] = control_channels[i][0];
						}
						if (control_channels[i][1] != SND_MIXER_SCHN_UNKNOWN &&
						    snd_mixer_selem_has_capture_channel(elem, control_channels[i][1])) {
							control->setFlags(control->getFlags() | HAS_CSWITCH_1);
							control->cswitch_channels[1] = control_channels[i][1];
						}
					}
				}
				if ((control->getFlags() & (HAS_VOLUME_0 | HAS_VOLUME_1)) == HAS_VOLUME_1) {
					control->setFlags(control->getFlags() ^ (HAS_VOLUME_0 | HAS_VOLUME_1));
					control->volume_channels[0] = control->volume_channels[1];
				}
				if ((control->getFlags() & (HAS_PSWITCH_0 | HAS_PSWITCH_1)) == HAS_PSWITCH_1) {
					control->setFlags(control->getFlags() ^ (HAS_PSWITCH_0 | HAS_PSWITCH_1));
					control->pswitch_channels[0] = control->pswitch_channels[1];
				}
				if ((control->getFlags() & (HAS_CSWITCH_0 | HAS_CSWITCH_1)) == HAS_CSWITCH_1) {
					control->setFlags(control->getFlags() ^ (HAS_CSWITCH_0 | HAS_CSWITCH_1));
					control->cswitch_channels[0] = control->cswitch_channels[1];
				}
				if (i == 0)
					front_control = control;
				else {
					front_control->setFlags(control->getFlags() | IS_MULTICH | 0);
					control->setFlags(control->getFlags() | IS_MULTICH | i);
				}
				controls.push_back(control);
			}
		}
	}
	if (viewMode != VIEW_MODE_PLAYBACK && (has_cvol || has_csw) && !merged_cswitch) {
		if ((!has_cvol || snd_mixer_selem_has_capture_volume_joined(elem)) &&
		    (!has_csw || snd_mixer_selem_has_capture_switch_joined(elem))) {
			std::shared_ptr<AlsaControl> control = std::make_shared<AlsaControl>(this, 0, 0);
			if (has_cvol) {
				control->setFlags(control->getFlags() | TYPE_CVOLUME | HAS_VOLUME_0);
				control->volume_channels[0] = SND_MIXER_SCHN_FRONT_LEFT;
			}
			if (has_csw) {
				control->setFlags(control->getFlags() | TYPE_CSWITCH | HAS_CSWITCH_0);
				control->cswitch_channels[0] = SND_MIXER_SCHN_FRONT_LEFT;
			}
			controls.push_back(control);
		} else {
			for (i = 0; i < ARRAY_SIZE(supported_channels); ++i)
				has_channel[supported_channels[i]] =
					snd_mixer_selem_has_capture_channel(elem, supported_channels[i]);
			for (i = 0; i < ARRAY_SIZE(control_channels); ++i) {
				has_ch0 = has_channel[control_channels[i][0]];
				has_ch1 = control_channels[i][1] != SND_MIXER_SCHN_UNKNOWN &&
					has_channel[control_channels[i][1]];
				if (!has_ch0 && !has_ch1)
					continue;
				auto control = std::make_shared<AlsaControl>(this, 0, 0);
				if (has_cvol) {
					control->setFlags(control->getFlags() | TYPE_CVOLUME);
					if (snd_mixer_selem_has_capture_volume_joined(elem)) {
						control->setFlags(control->getFlags()  | HAS_VOLUME_0);
						control->volume_channels[0] = SND_MIXER_SCHN_FRONT_LEFT;
					} else {
						if (has_ch0) {
							control->setFlags(control->getFlags() | HAS_VOLUME_0);
							control->volume_channels[0] = control_channels[i][0];
						}
						if (has_ch1) {
							control->setFlags(control->getFlags() | HAS_VOLUME_1);
							control->volume_channels[1] = control_channels[i][1];
						}
					}
				}
				if (has_csw) {
					control->setFlags(control->getFlags() | TYPE_CSWITCH);
					if (snd_mixer_selem_has_capture_switch_joined(elem)) {
						control->setFlags(control->getFlags() | HAS_CSWITCH_0);
						control->cswitch_channels[0] = SND_MIXER_SCHN_FRONT_LEFT;
					} else {
						if (has_ch0) {
							control->setFlags(control->getFlags() | HAS_CSWITCH_0);
							control->cswitch_channels[0] = control_channels[i][0];
						}
						if (has_ch1) {
							control->setFlags(control->getFlags() | HAS_CSWITCH_1);
							control->cswitch_channels[1] = control_channels[i][1];
						}
					}
				}
				if ((control->getFlags() & (HAS_VOLUME_0 | HAS_VOLUME_1)) == HAS_VOLUME_1) {
					control->setFlags(control->getFlags() ^ (HAS_VOLUME_0 | HAS_VOLUME_1));
					control->volume_channels[0] = control->volume_channels[1];
				}
				if ((control->getFlags() & (HAS_CSWITCH_0 | HAS_CSWITCH_1)) == HAS_CSWITCH_1) {
					control->setFlags(control->getFlags() ^ (HAS_CSWITCH_0 | HAS_CSWITCH_1));
					control->cswitch_channels[0] = control->cswitch_channels[1];
				}
				if (i == 0)
					front_control = control;
				else {
					front_control->setFlags(control->getFlags() | IS_MULTICH | 0);
					control->setFlags(control->getFlags() | IS_MULTICH | i);
				}
				controls.push_back(control);
			}
		}
	}

	//std::cout << "MixerElement::create_controls_for_elem " << controls.size() << std::endl;
	//for (auto cntl : controls) {
	//	std::cout << "  cntl " << cntl->getInfo() << std::endl;
	//}
	return controls;
}


static bool has_more_than_front_capture_channels(snd_mixer_elem_t *elem)
{
	unsigned int i;

	for (i = 2; i < ARRAY_SIZE(supported_channels); ++i)
		if (snd_mixer_selem_has_capture_channel(elem, supported_channels[i]))
			return true;
	return false;
}

static bool has_any_control_channel(snd_mixer_elem_t *elem,
				    const snd_mixer_selem_channel_id_t channels[2],
				    int (*has_channel)(snd_mixer_elem_t *, snd_mixer_selem_channel_id_t))
{
	return has_channel(elem, channels[0]) ||
	       (channels[1] != SND_MIXER_SCHN_UNKNOWN && has_channel(elem, channels[1]));
}

bool
AlsaElement::has_merged_cswitch(snd_mixer_elem_t *elem)
{
	bool pvol, psw;
	unsigned int i;

	pvol = snd_mixer_selem_has_playback_volume(elem);
	psw = snd_mixer_selem_has_playback_switch(elem);
	if ((pvol || psw) &&
	    snd_mixer_selem_has_capture_switch(elem) &&
	    !snd_mixer_selem_has_capture_volume(elem)) {
		if (snd_mixer_selem_has_capture_switch_joined(elem))
			return true;
		else if (((pvol && snd_mixer_selem_has_playback_volume_joined(elem)) ||
			  (psw && snd_mixer_selem_has_playback_switch_joined(elem))) &&
			 has_more_than_front_capture_channels(elem))
			return false;
		for (i = 0; i < ARRAY_SIZE(control_channels); ++i) {
			if (has_any_control_channel(elem, control_channels[i], snd_mixer_selem_has_capture_channel) &&
			    !has_any_control_channel(elem, control_channels[i], snd_mixer_selem_has_playback_channel))
				return false;
		}
		return true;
	}
	return false;
}

snd_mixer_elem_t *
AlsaElement::getMixerElement()
{
	return m_elem;
}

void
AlsaElement::update()
{
	m_notifySignal.emit();
}

AlsaElement::NotfiySignal&
AlsaElement::getNotifications()
{
	return m_notifySignal;
}
