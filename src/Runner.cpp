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

#include <QCommandLineParser>
#include <QCommandLineOption>

#include "main_window.h"

Runner::Runner(int argc, char **argv, QString dispname, QString appname, QString appver, QString orgname, QString orgdomain) {
    //Q_INIT_RESOURCE(adtool);

    this->argc = argc;
    this->argv = argv;

    this->app = new QApplication(this->argc, this->argv);
    this->app->setApplicationDisplayName(dispname);
    this->app->setApplicationName(appname);
    this->app->setApplicationVersion(appver);
    this->app->setOrganizationName(orgname);
    this->app->setOrganizationDomain(orgdomain);
}

void Runner::arg_parser() {
    QCommandLineParser cli_parser;
    cli_parser.setApplicationDescription(QCoreApplication::applicationName());
    cli_parser.addHelpOption();
    cli_parser.addVersionOption();
    const QStringList arg_list = qApp->arguments();
    cli_parser.process(arg_list);
}

int Runner::run() {
    this->arg_parser();

    MainWindow main_window;
    main_window.show();

    return app->exec();
}

