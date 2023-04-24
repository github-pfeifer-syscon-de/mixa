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

#include <gtkmm.h>
#include <memory>

#include "AlsaMixer.hpp"
#include "AlsaCard.hpp"

class CardFrame : public Gtk::Box {
public:
    CardFrame(const std::shared_ptr<AlsaCard>& card);
    virtual ~CardFrame();
    void addModePages(Gtk::Notebook* playCaptNotebook);
    std::shared_ptr<AlsaCard> getCard();
    void on_show() override;
    void on_hide() override;
    std::shared_ptr<AlsaMixer> getMixer();
private:
    void selectMixer();

    std::shared_ptr<AlsaCard> m_card;
    std::shared_ptr<AlsaMixer> m_mixer;
};

