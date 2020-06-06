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
#include "main_window.h"
#include "admc.h"

#include <QCommandLineParser>
#include <QCommandLineOption>

Runner::Runner(int& argc_, char **argv_, QString dispname, QString appname, QString appver, QString orgname, QString orgdomain) {
    //Q_INIT_RESOURCE(adtool);

    this->argc = argc_;
    this->argv = argv_;

    this->app = new ADMC(argc, argv);
    this->app->setApplicationDisplayName(dispname);
    this->app->setApplicationName(appname);
    this->app->setApplicationVersion(appver);
    this->app->setOrganizationName(orgname);
    this->app->setOrganizationDomain(orgdomain);
}

int Runner::run() {
    QCommandLineParser cli_parser;
    cli_parser.setApplicationDescription(QCoreApplication::applicationName());
    cli_parser.addHelpOption();
    cli_parser.addVersionOption();
    const QStringList arg_list = qApp->arguments();

    QCommandLineOption option_auto_login("a", "Automatically login to default domain on startup");
    cli_parser.addOption(option_auto_login);

    cli_parser.process(arg_list);

    const bool auto_login = cli_parser.isSet(option_auto_login);

    MainWindow main_window(auto_login);
    main_window.show();

    return app->exec();
}

