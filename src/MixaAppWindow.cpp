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
#include <thread>
#include <future>
#include <gtkmm.h>
#include <math.h>

#include "MixaAppWindow.hpp"
#include "MixaApp.hpp"
#include "AlsaCard.hpp"
#include "AlsaMixer.hpp"
#include "AlsaControl.hpp"
#ifdef KEYBINDER
#include "Keybind.hpp"
#endif


MixaAppWindow::MixaAppWindow(Gtk::Application* application)
: Gtk::ApplicationWindow{}
, m_application{application}
, m_keybind{nullptr}
, m_cardNotebook{nullptr}
{
    Glib::RefPtr<Gdk::Pixbuf> pix = Gdk::Pixbuf::create_from_resource(
                                                                      application->get_resource_base_path() + "/mixa.png");
    set_icon(pix);

    m_cardNotebook = Gtk::manage(new Gtk::Notebook());
    add(*m_cardNotebook);

    set_default_size(512, 384);
    for (auto card : AlsaCard::getCards())
    {
        auto cardPage = Gtk::manage(new CardFrame(card));
        m_cardNotebook->append_page(*cardPage, card->getDeviceName(), false);
    }

    show_all_children();
#ifdef KEYBINDER
	m_keybind = new Keybind();
	m_keybind->getNotification().connect(sigc::mem_fun(*this, &MixaAppWindow::key_notify));
#endif
}

MixaAppWindow::~MixaAppWindow()
{
#ifdef KEYBINDER
	if (m_keybind) {
		delete m_keybind;
	}
#endif
}

void
MixaAppWindow::key_notify()
{
#ifdef KEYBINDER
	KEYTYPE keytype = m_keybind->getKey();
	if (keytype != KEYTYPE::NONE) {
		std::cout << "key " << (int)keytype << std::endl;
		CardFrame* cardFrame = (CardFrame *)m_cardNotebook->get_nth_page(m_cardNotebook->get_current_page());
		if (cardFrame != nullptr) {
			auto mixer = cardFrame->getMixer();
			auto elements = mixer->getElements();
			if (!elements.empty()) {
				auto element = elements.front();
				auto controls = element->getPlayControls();
				if (!controls.empty()) {
					auto control = controls.front();
					if (control->hasVolume()) {
						double vol = control->getVolume();
						if (keytype == KEYTYPE::RAISE_VOLUME) {
							vol = std::min(vol + 10.0, 100.0);
							control->setVolume(vol);
						}
						else if (keytype == KEYTYPE::LOWER_VOLUME) {
							vol = std::max(vol - 10.0, 0.0);
							control->setVolume(vol);
						}
					}
					if (control->hasMute()) {
						if (keytype == KEYTYPE::MUTE) {
							control->setMute(!control->getMute());
						}
					}
				}
			}
		}
	}
#endif
}

void
MixaAppWindow::on_action_preferences()
{
    //Gtk::Dialog *dlg = m_monglView->monitors_config();
    //dlg->set_transient_for(*this);
    //dlg->run();
    //m_monglView->save_config();
    //dlg->hide();
    //delete dlg;
}

void
MixaAppWindow::on_action_about()
{
    auto refBuilder = Gtk::Builder::create();
    try
    {
        refBuilder->add_from_resource(m_application->get_resource_base_path() + "/abt-dlg.ui");
        auto object = refBuilder->get_object("abt-dlg");
        auto abtdlg = Glib::RefPtr<Gtk::AboutDialog>::cast_dynamic(object);
        if (abtdlg)
        {
            Glib::RefPtr<Gdk::Pixbuf> pix = Gdk::Pixbuf::create_from_resource(
                                                                              m_application->get_resource_base_path() + "/mixa.png");
            abtdlg->set_logo(pix);
            abtdlg->set_transient_for(*this);
            abtdlg->run();
            abtdlg->hide();
        }
        else
            std::cerr << "MixaAppWindow::on_action_about(): No \"abt-dlg\" object in abt-dlg.ui"
                << std::endl;
    }
    catch (const Glib::Error& ex)
    {
        std::cerr << "MixaAppWindow::on_action_about(): " << ex.what() << std::endl;
    }

}

