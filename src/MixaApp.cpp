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



#include <gtkmm.h>
#include <iostream>
#include <exception>
#include <thread>
#include <future>

#include "MixaAppWindow.hpp"
#include "MixaApp.hpp"

MixaApp::MixaApp(int argc, char **argv)
: Gtk::Application(argc, argv, "de.pfeifer_syscon.mixa")
, m_mixaAppWindow(nullptr)
{

}

MixaApp::MixaApp(const MixaApp& orig)
: m_mixaAppWindow(orig.m_mixaAppWindow)
{
}

MixaApp::~MixaApp()
{
    //std::cout << "MixaApp::~MixaApp" << std::endl;
}

void
MixaApp::on_activate()
{
    add_window(*m_mixaAppWindow);
    m_mixaAppWindow->show();
}


void
MixaApp::on_action_quit()
{
    //std::cout << "MixaApp::on_action_quit" << std::endl;
    m_mixaAppWindow->hide();

    // Not really necessary, when Gtk::Widget::hide() is called, unless
    // Gio::Application::hold() has been called without a corresponding call
    // to Gio::Application::release().
    //std::cout << "quit " << std::endl;
    quit();
}

void
MixaApp::on_shutdown()
{
    // this is important as we crash otherwise on exit
    if (m_mixaAppWindow) {
        delete m_mixaAppWindow;
        m_mixaAppWindow = nullptr;
    }
}

void
MixaApp::on_startup()
{
    // Call the base class's implementation.
    Gtk::Application::on_startup();
    m_mixaAppWindow = new MixaAppWindow(this);
    signal_shutdown().connect(sigc::mem_fun(*this, &MixaApp::on_shutdown));

    // Add actions and keyboard accelerators for the application menu.
    add_action("preferences", sigc::mem_fun(*m_mixaAppWindow, &MixaAppWindow::on_action_preferences));
    add_action("about", sigc::mem_fun(*m_mixaAppWindow, &MixaAppWindow::on_action_about));
    add_action("quit", sigc::mem_fun(*this, &MixaApp::on_action_quit));
    set_accel_for_action("app.quit", "<Ctrl>Q");

    auto refBuilder = Gtk::Builder::create();
    try {
        refBuilder->add_from_resource(get_resource_base_path() + "/app-menu.ui");
        auto object = refBuilder->get_object("appmenu");
        auto app_menu = Glib::RefPtr<Gio::MenuModel>::cast_dynamic(object);
        if (app_menu)
            set_app_menu(app_menu);
        else
            std::cerr << "MixaApp::on_startup(): No \"appmenu\" object in app_menu.ui"
                << std::endl;
    } catch (const Glib::Error& ex) {
        std::cerr << "MixaApp::on_startup(): " << ex.what() << std::endl;
        return;
    }
}

int main(int argc, char** argv) {
    auto app = MixaApp(argc, argv);

    return app.run();
}