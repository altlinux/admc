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

#if !defined(__ADMC_RUNNER_H)
#define __ADMC_RUNNER_H 1

#include "ad_connection.h"
#include "Application.h"

#include <QString>

class Runner {
    ADMC* app;
    int argc;
    char** argv;

public:
    Runner(int& argc, char **argv, QString dispname, QString appname, QString appver, QString orgname, QString orgdomain);
    void arg_parser();
    int run();
};

#endif /* __ADMC_RUNNER_H */

