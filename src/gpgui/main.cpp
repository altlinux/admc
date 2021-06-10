/*
 * GPGUI - Group Policy Editor GUI
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
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

#include "MainWindow.h"
#include "config.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QStringList>

#define CLI_OPTION_PATH "path"

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    app.setApplicationDisplayName(GPGUI_APPLICATION_DISPLAY_NAME);
    app.setApplicationName(GPGUI_APPLICATION_NAME);
    app.setApplicationVersion(GPGUI_VERSION);
    app.setOrganizationName(GPGUI_ORGANIZATION);
    app.setOrganizationDomain(GPGUI_ORGANIZATION_DOMAIN);

    QTranslator translator;
    translator.load(QLocale(), QString(), QString(), ":/translations");
    app.installTranslator(&translator);

    QCommandLineParser cli_parser;
    cli_parser.setApplicationDescription(QCoreApplication::applicationName());
    cli_parser.addHelpOption();

    cli_parser.addOption({{"p", "path"}, "Policy's share path to use, it's in the gPCFileSysPath attribute of the LDAP object", CLI_OPTION_PATH});

    const QStringList arg_list = qApp->arguments();
    cli_parser.process(arg_list);

    const QString path = cli_parser.value(CLI_OPTION_PATH);

    MainWindow main_window(path);
    main_window.show();

    const int retval = app.exec();

    return retval;
}
