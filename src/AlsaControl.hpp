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

class AlsaElement;

#define TYPE_PVOLUME	(1u << 4)
#define TYPE_CVOLUME	(1u << 5)
#define TYPE_PSWITCH	(1u << 6)
#define TYPE_CSWITCH	(1u << 7)
#define TYPE_ENUM	(1u << 8)
#define HAS_VOLUME_0	(1u << 9)
#define HAS_VOLUME_1	(1u << 10)
#define HAS_PSWITCH_0	(1u << 11)
#define HAS_PSWITCH_1	(1u << 12)
#define HAS_CSWITCH_0	(1u << 13)
#define HAS_CSWITCH_1	(1u << 14)
#define IS_MULTICH	(1u << 15)
#define IS_ACTIVE	(1u << 16)
#define MULTICH_MASK	(0x0000f)


class AlsaControl {
public:
    AlsaControl(AlsaElement *mixerElement, uint32_t flags, uint32_t channel_bits);
    virtual ~AlsaControl();

    void setFlags(uint32_t flags);
    uint32_t getFlags() ;

    bool hasVolume();
    double getVolume();
    void setVolume(double value);
    std::string getName();
    std::string getInfo();
    bool hasMute();
    bool getMute();
    void setMute(bool muted);
    AlsaElement* getMixerElement();

    snd_mixer_selem_channel_id_t volume_channels[2];
    snd_mixer_selem_channel_id_t pswitch_channels[2];
    snd_mixer_selem_channel_id_t cswitch_channels[2];
    std::string getShortName();
protected:
    std::string getMultiChannelName();
    std::string getChannelName(snd_mixer_selem_channel_id_t ch);
private:
    void replace(std::string& str, const std::string& oldStr, const std::string& newStr);

    AlsaElement* m_mixerElement;
    uint32_t m_flags;
    uint32_t m_channel_bits;
    uint32_t m_enum_channel_bits;
    std::string m_name;
    std::string m_shortName;
    uint32_t m_enum_index;
};

