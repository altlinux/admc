
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

#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QTranslator>
#include <QGuiApplication>

int main(int argc, char **argv) {
    Q_INIT_RESOURCE(adldap);

    QGuiApplication::setDesktopFileName("admc");

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

    std::unique_ptr<Krb5Client> krb5_client = nullptr;
    try {
        krb5_client = std::unique_ptr<Krb5Client>(new Krb5Client);

        const QString last_logged_user = settings_get_variant(SETTING_last_logged_user).toString();
        if (!last_logged_user.isEmpty() && krb5_client->active_tgt_principals().contains(last_logged_user)) {
            krb5_client->set_current_principal(last_logged_user);
        }
    }
    catch (const std::runtime_error& e) {
        qWarning(e.what());
    }

    load_connection_options();

    MainWindow *main_window = nullptr;
    {
        const bool show_login_window = settings_get_variant(SETTING_show_login_window_on_startup).toBool();
        if (show_login_window) {
            krb5_client->logout(false);
        }

        AdInterface ad;
        main_window = new MainWindow(ad, *krb5_client);
        main_window->show();

        if (ad.is_connected()) {
            main_window->show_changelog_on_update();
        }
        else {
            main_window->open_auth_dialog();
        }
    }

    const int retval = app.exec();

    delete main_window;

    return retval;
}
