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
#include "Runner.h"

#include <memory>

int main(int argc, char **argv) {
    std::unique_ptr<Runner> runner(new Runner(argc,
        argv,
        ADMC_APPLICATION_DISPLAY_NAME,
        ADMC_APPLICATION_NAME,
        ADMC_VERSION,
        ADMC_ORGANIZATION,
        ADMC_ORGANIZATION_DOMAIN));

    return runner->run();
}
