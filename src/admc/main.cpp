/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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
#include "globals.h"
#include "main_window.h"
#include "settings.h"

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

    const QLocale saved_locale = g_settings->get_variant(VariantSetting_Locale).toLocale();

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
    const bool loaded_qt_translation = qt_translator.load("qt_" + saved_locale.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qt_translator);

    if (!loaded_qt_translation) {
        qDebug() << "Failed to load qt translation";
    }

    MainWindow main_window;
    main_window.show();

    const int retval = app.exec();

    return retval;
}
