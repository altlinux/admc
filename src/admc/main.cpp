
/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QGuiApplication>
#include <QScreen>

#include "adldap.h"
#include "ui/dialog/connection_options.h"
#include "core/admc_translator.h"
#include "core/config.h"
#include "core/globals.h"
#include "core/settings.h"
#include "locale.h"
#include "ui/main_window.h"
#include "ui/dialog/main_window_connection_error.h"
#include "ui/status.h"
#include "ui/utils.h"

int main(int argc, char **argv) {
    Q_INIT_RESOURCE(adldap);

    qputenv("QT_SCALE_FACTOR", "1");

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

    QScreen *screen = QGuiApplication::primaryScreen();
    qreal dpi = screen->logicalDotsPerInch();
    QFont font = app.font();
    // 96 DPI = base size
    int baseSize = 11;
    int scaledSize = std::round(baseSize * dpi / 96.0);
    font.setPointSize(scaledSize);
    app.setFont(font);

    AdmcTranslator::get_instance().load_saved_locale();

    std::unique_ptr<Krb5Client> krb5_client = nullptr;
    try {
        krb5_client = std::unique_ptr<Krb5Client>(new Krb5Client);

        const QString last_logged_user =
            settings_get_string(SETTING_last_logged_user);
        if (!last_logged_user.isEmpty() && krb5_client->active_tgt_principals().contains(last_logged_user)) {
            krb5_client->set_current_principal(last_logged_user);
        }
    }
    catch (const std::runtime_error& e) {
        qWarning(e.what());
    }

    load_connection_options();

    MainWindow *main_window = nullptr;
    MainWindowConnectionError *error_window = nullptr;
    {
        const bool show_login_window =
            settings_get_bool(SETTING_show_login_window_on_startup);
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
            bool no_principals = krb5_client->active_tgt_principals().isEmpty();
            if (show_login_window || no_principals) {
                main_window->open_auth_dialog();
            } else {
                error_window = new MainWindowConnectionError(main_window);
                error_window->show();
            }
        }
    }

    const int retval = app.exec();

    delete main_window;

    return retval;
}
