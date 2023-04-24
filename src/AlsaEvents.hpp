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
#include <thread>
#include <atomic>
#include <glibmm.h>

// as this behaviour has its own issues encapsulate it
class AlsaEvents {
public:
    AlsaEvents(snd_mixer_t* mixer);
    virtual ~AlsaEvents();

    virtual void destroy();
protected:
    std::atomic<bool> m_pollActive; // Use a atomic flag, as a bool will not be updated even using volatile ...
    snd_mixer_t* m_mixer;
private:

};

// this keeps it in on main-thread
class AlsaIdleEvents : public AlsaEvents {
public:
    AlsaIdleEvents(snd_mixer_t* mixer);
    virtual ~AlsaIdleEvents();

    void destroy() override;
protected:
    bool pollIdle();
private:
    sigc::connection m_idleConnect;
    struct pollfd* m_pollfds;
    int m_nfds;
};

// this had some issues with raspi/usb-card (sb extigy element with multiple controls)
//   might be solved by using dispatch here ?
class AlsaThreadEvents : public AlsaEvents {
public:
    AlsaThreadEvents(snd_mixer_t* mixer);
    virtual ~AlsaThreadEvents();

    void destroy() override;
protected:
    void pollThread();
    void handleMixerEvents();
private:
    std::thread *m_workerThread;
    Glib::Dispatcher m_Dispatcher;
    struct pollfd* m_pollfds;
    int m_nfds;
};
