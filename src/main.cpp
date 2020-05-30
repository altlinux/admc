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

#include "main_window.h"
#include "ad_interface.h"

#include <QApplication>

int main(int argc, char **argv) {
    // Load fake AD data if given "fake" argument
    // This also swaps all ad interface functions to their fake versions (including login)
    if (argc >= 1 && QString(argv[1]) == "fake") {
        FAKE_AD = true;
    }

    if (!ad_interface_login()) {
        return 1;
    }

    QApplication app(argc, argv);
    MainWindow main_window;
    main_window.show();

    const int retval = app.exec();

    return retval;
}
