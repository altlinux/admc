/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "config.h"

#include "admc.h"

ADMC* ADMC::instance;

ADMC::ADMC(int& argc, char** argv)
: QApplication(argc, argv)
{
    connection = new adldap::AdConnection();
}

ADMC *ADMC::create(int& argc, char** argv) {
    if (instance == nullptr) {
        instance = new ADMC(argc, argv);

        return instance;
    } else {
        printf("Error: calling ADMC::create() twice!");

        return instance;
    }
}

ADMC* ADMC::get_instance() {
    if (instance == nullptr) {
        printf("Error: calling ADMC::get_instance() before ADMC::create!");

        return nullptr;
    } else {
        return instance;
    }
}

adldap::AdConnection* ADMC::get_connection() {
    return connection;
}

