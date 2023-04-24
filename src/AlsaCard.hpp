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

#include <list>
#include <string>
#include <memory>

class AlsaCard {
public:
    AlsaCard(const std::string& _indexstr, const std::string& _name, const std::string& _device_name);
    virtual ~AlsaCard();

    static std::list<std::shared_ptr<AlsaCard>> getCards();
    std::string getIndexstr();
    std::string getName();
    std::string getDeviceName();
private:
    std::string m_indexstr;
    std::string m_name;
    std::string m_device_name;
};

