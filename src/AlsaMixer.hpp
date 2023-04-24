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
#include <string>
#include <list>
#include <memory>


#include "AlsaElement.hpp"
class AlsaEvents;
class AlsaCard;

class AlsaMixer {
public:
    AlsaMixer(std::shared_ptr<AlsaCard>& card);
    virtual ~AlsaMixer();

    void attach(const std::string& card_name);
    bool isUnplugged(); // indicates if usable
    std::list<std::shared_ptr<AlsaElement>> getElements();
    static void addToAllMixers(const std::shared_ptr<AlsaMixer>& mixer);
    void removeFromAllMixers();
protected:
    //static bool s_controls_changed;
    //static bool s_control_values_changed;
private:
    void createElement(snd_mixer_elem_t *elem);
    static int mixer_callback(snd_mixer_t *mixer, unsigned int mask, snd_mixer_elem_t *elem);
    static int elem_callback(snd_mixer_elem_t *elem, unsigned int mask);
    //static std::list<std::shared_ptr<AlsaElement>> s_allElements;
    static std::list<std::shared_ptr<AlsaMixer>> s_allMixers;
    void close_hctl();

    snd_mixer_t* m_mixer;
    std::string m_mixer_device_name;
    bool m_unplugged;
    std::list<std::shared_ptr<AlsaElement>> m_elements;
    AlsaEvents* m_alsaEvents;
};

