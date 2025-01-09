/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "adldap.h"
#include "config.h"
#include "connection_options_dialog.h"
#include "globals.h"
#include "main_window.h"
#include "main_window_connection_error.h"
#include "settings.h"
#include "status.h"
#include "utils.h"
#include "connection_options_dialog.h"
#include "locale.h"
#include "managers/icon_manager.h"

#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QTranslator>

int main(int argc, char **argv) {
    Q_INIT_RESOURCE(adldap);

    // NOTE: this is needed to pass this type from thread's
    // signal in find_widget.cpp. Without doing this,
    // passing this type from thread results in a runtime
    // error.
    qRegisterMetaType<QHash<QString, AdObject>>("QHash<QString, AdObject>");

    QApplication app(argc, argv);
    app.setApplicationDisplayName(ADMC_APPLICATION_DISPLAY_NAME);
    app.setApplicationName(ADMC_APPLICATION_NAME);
    app.setApplicationVersion(ADMC_VERSION);
    app.setOrganizationName(ADMC_ORGANIZATION);
    app.setOrganizationDomain(ADMC_ORGANIZATION_DOMAIN);
    app.setWindowIcon(QIcon(":/admc/admc.ico"));

    const QLocale saved_locale = settings_get_variant(SETTING_locale).toLocale();
    const QString locale_dot_UTF8 = saved_locale.name() + ".UTF-8";
    const char* locale_for_c = std::setlocale(LC_ALL, locale_dot_UTF8.toLocal8Bit().data());
    if (!locale_for_c) {
        qDebug() << "Failed to set locale for C libs";
    }

    QTranslator translator;
    const bool loaded_admc_translation = translator.load(saved_locale, "admc", "_", ":/admc");
    app.installTranslator(&translator);

    if (!loaded_admc_translation) {
        qDebug() << "Failed to load admc translation";
    }

    QTranslator adldap_translator;
    const bool loaded_adldap_translation = load_adldap_translation(adldap_translator, saved_locale);
    app.installTranslator(&adldap_translator);

    if (!loaded_adldap_translation) {
        qDebug() << "Failed to load adldap translation";
    }

    // NOTE: these translations are for qt-defined text, like standard dialog buttons
    QTranslator qt_translator;
    const bool loaded_qt_translation = qt_translator.load(saved_locale, "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qt_translator);

    if (!loaded_qt_translation) {
        qDebug() << "Failed to load qt translation";
    }

    QTranslator qtbase_translator;
    const bool loaded_qtbase_translation = qtbase_translator.load(saved_locale, "qtbase", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtbase_translator);

    if (!loaded_qtbase_translation) {
        qDebug() << "Failed to load qt base translation";
    }

    load_connection_options();

    // In case of failure to connect to AD and load
    // adconfig, we open a special alternative main window.
    // We do this to acomplish 2 objectives:
    // * First, we check that connection is possible at
    //   startup and give the user an opportunity to
    //   troubleshoot by adjusting connection options
    //   (available in the alternative main window).
    // * Secondly, we guarantee that the real MainWindow is
    //   created only after adconfig has been loaded. This
    //   is needed because many child widgets used by
    //   MainWindow require adconfig to load their UI
    //   elements.
    //
    // Control flow here is awkward for multiple
    // reasons. First, the ad error log has to be
    // displayed *after* main window is shown and
    // parented to it to be modal. Secondly, we need
    // adinterface to stay in this scope so that it is
    // destroyed when scope is over. If it's created
    // outside the scope, then it will stay alive for
    // the whole duration of the app which is bad.
    QMainWindow *first_main_window = nullptr;
    {
        AdInterface ad;

        if (ad.is_connected()) {
            load_g_adconfig(ad);

            first_main_window = new MainWindow(ad);
            first_main_window->show();
        } else {
            first_main_window = new MainWindowConnectionError();
            first_main_window->show();

            ad_error_log(ad, first_main_window);
        }
    }

    const int retval = app.exec();

    delete first_main_window;

    return retval;
}
