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

#include <signal.h>
#include <iostream>
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <sys/signalfd.h>
#include <bits/sigthread.h>

#include "AlsaEvents.hpp"

AlsaEvents::AlsaEvents(snd_mixer_t* mixer)
: m_pollActive{true}
, m_mixer{mixer}
{
}

AlsaEvents::~AlsaEvents()
{
}

void
AlsaEvents::destroy()
{
	m_pollActive = false;
}

AlsaIdleEvents::AlsaIdleEvents(snd_mixer_t* mixer)
: AlsaEvents::AlsaEvents{mixer}
, m_idleConnect{}
, m_pollfds{nullptr}
, m_nfds{0}
{
	const auto idle_source = Glib::IdleSource::create();
	m_idleConnect = idle_source->connect(sigc::mem_fun(*this, &AlsaIdleEvents::pollIdle));
	idle_source->attach(Glib::MainContext::get_default());
}

AlsaIdleEvents::~AlsaIdleEvents()
{
	free(m_pollfds);
}

void
AlsaIdleEvents::destroy()
{
	AlsaEvents::destroy();
	if (m_idleConnect.connected()) {	// ensure no more calls
		m_idleConnect.disconnect();
	}
}

bool
AlsaIdleEvents::pollIdle()
{
	//int n = snd_mixer_poll_descriptors_count(m_mixer);
	//if (n != m_nfds) {
	//	free(m_pollfds);
	//	m_nfds = n;
	//	m_pollfds = (struct pollfd*)calloc(m_nfds, sizeof *m_pollfds);
	//}
	//int err = snd_mixer_poll_descriptors(m_mixer, m_pollfds, m_nfds);
	//if (err < 0) {
	//	std::cerr << "cannot get poll descriptors "
	//			  << " err " << err
	//			  << " strerr " << snd_strerror(err) << std::endl;
	//	return false;
	//}
	//n = poll(m_pollfds, m_nfds, 10u);	// wait just for a short period as we are on main thread
	//if (n < 0) {
	//	if (errno == EINTR) {
	//		// don't take this too serious
	//	}
	//	else {
	//		std::cerr << "poll error "
	//			      << " err " <<  n
	//			      << " errno " << errno << std::endl;
	//		return false;
	//	}
	//}
	//int n = 1;	// try to skip using descriptors
	//if (n > 0) {
	//	unsigned short revents;
	//	int err = snd_mixer_poll_descriptors_revents(m_mixer, m_pollfds, m_nfds, &revents);
	//	//std::cout << "n " << n << " from poll err " << err << " revents " << revents << std::endl;
	//	if (err < 0) {
	//		std::cerr <<  "cannot get poll events "
	//				  << " err " << err
	//				  << " strerr " << snd_strerror(err) << std::endl;
	//		return false;
	//	}
	//	else if (revents & (POLLERR | POLLNVAL)) {
	//		//close_mixer_device();
	//		std::cerr <<  "shoud close_mixer_device "
	//				  << " err " << err << std::endl;
	//		return false;
	//	}
	//	else if (revents & POLLIN) {
			snd_mixer_handle_events(m_mixer);
	//	}
	//}

	return m_pollActive;
}

AlsaThreadEvents::AlsaThreadEvents(snd_mixer_t* mixer)
: AlsaEvents::AlsaEvents{mixer}
, m_workerThread{nullptr}
, m_Dispatcher{}
, m_pollfds{nullptr}
, m_nfds{0}
{
	m_Dispatcher.connect(sigc::mem_fun(*this, &AlsaThreadEvents::handleMixerEvents));
	m_workerThread = new std::thread(sigc::mem_fun(*this, &AlsaThreadEvents::pollThread));
}

AlsaThreadEvents::~AlsaThreadEvents()
{
}

void
AlsaThreadEvents::pollThread()
{
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);	// this signal we want to use to terminate thread
	int addNfds = 0;
	auto signal_fd = signalfd(-1, &set, 0);	// create file descriptor for signal
	if (signal_fd >= 0) {
		int ret = pthread_sigmask(SIG_BLOCK, &set, NULL);	// as we want to handle intr block default (as we want to free our resources)
		if (ret == 0) {
			addNfds = 1;	// preparation went ok we can use a filedescr. to catch signal
		}
		else {
			std::cout << "Error " << ret << " changing thread mask" << std::endl;
		}
	}
	else {
		std::cout << "Error " << signal_fd << " get signalfd" << std::endl;
	}
	while (m_pollActive) {
		int n = snd_mixer_poll_descriptors_count(m_mixer);
		if (n != m_nfds) {
			free(m_pollfds);
			m_nfds = n;
			m_pollfds = (struct pollfd*)calloc(m_nfds+addNfds, sizeof *m_pollfds);
		}
		int err = snd_mixer_poll_descriptors(m_mixer, m_pollfds, m_nfds);
		if (err < 0) {
			std::cerr << "cannot get poll descriptors "
					  << " err " << err
					  << " strerr " << snd_strerror(err) << std::endl;
			break;
		}
		if (addNfds > 0) {
			m_pollfds[m_nfds] = {
				.fd = signal_fd,
				.events = POLLIN };
		}
		n = poll(m_pollfds, m_nfds+addNfds, -1);	// here we can wait indefinitely
		if (n < 0) {
			if (errno == EINTR) {	// if we get a interrupt
				break;
			}
			else {
				std::cerr << "poll error " 
					      << " err " << n
					      << " errno " << errno << std::endl;
			}
		}
		if (n > 0) {
			m_Dispatcher.emit(); // escape thread box early
		}
	}
	//std::cout << "AlsaThreadEvents::pollThread exit" << std::endl;
	if (signal_fd >= 0) {
		close(signal_fd);
	}
	free(m_pollfds);
}

void
AlsaThreadEvents::handleMixerEvents()
{
	// try to put this behind dispatch (seems to solve thread issues)
	unsigned short revents;
	int err = snd_mixer_poll_descriptors_revents(m_mixer, m_pollfds, m_nfds, &revents);
	//std::cout << "n " << n << " from poll err " << err << " revents " << revents << std::endl;
	if (err < 0) {
		std::cerr << "cannot get poll events "
				  << " err " << err
				  << " strerr " << snd_strerror(err) << std::endl;
	}
	else if (revents & (POLLERR | POLLNVAL)) {
		//close_mixer_device();
		std::cerr << "shoud close_mixer_device "
				  << err << std::endl;
		//break;
	}
	else if (revents & POLLIN) {
		snd_mixer_handle_events(m_mixer);
	}
}

void
AlsaThreadEvents::destroy()
{
	AlsaEvents::destroy();
	if (m_workerThread
		&& m_workerThread->joinable()) {
		//std::cout << "AlsaThreadEvents::close signal  " << std::endl;
		int err = pthread_kill(m_workerThread->native_handle(), SIGINT);
		if (err < 0) {
			std::cout << "AlsaThreadEvents::close on pthread_kill"
				      << " err " << err
				      << " errno " << errno << std::endl;
		}
		//std::cout << "AlsaThreadEvents::close join entry" << std::endl;
		m_workerThread->join();
		//std::cout << "AlsaThreadEvents::close join exit" << std::endl;

		m_workerThread = nullptr;
	}
}