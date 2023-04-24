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

#include <alsa/asoundlib.h>
#include <list>
#include <memory>
#include <glibmm.h>

class AlsaControl;
enum view_mode {
	VIEW_MODE_PLAYBACK,
	VIEW_MODE_CAPTURE,
	VIEW_MODE_ALL,
	VIEW_MODE_COUNT,
};
#define LAST_SUPPORTED_CHANNEL SND_MIXER_SCHN_SIDE_RIGHT

#define ARRAY_SIZE(a) (sizeof(a) / sizeof *(a))

class AlsaElement {
public:
    AlsaElement(snd_mixer_elem_t *elem);
    virtual ~AlsaElement();

    snd_mixer_elem_t *getMixerElement();
    std::list<std::shared_ptr<AlsaControl>> getPlayControls();
    std::list<std::shared_ptr<AlsaControl>> getCaptureControls();
    void update();
    using NotfiySignal = sigc::signal<void>;
    NotfiySignal& getNotifications();
protected:

    bool has_merged_cswitch(snd_mixer_elem_t *elem);

    std::list<std::shared_ptr<AlsaControl>> create_controls_for_elem(view_mode viewMode);
private:
    snd_mixer_selem_id_t* m_selem_id;
    snd_mixer_elem_t* m_elem;
    std::list<std::shared_ptr<AlsaControl>> m_playControls;
    std::list<std::shared_ptr<AlsaControl>> m_captControls;
    NotfiySignal m_notifySignal;

};

