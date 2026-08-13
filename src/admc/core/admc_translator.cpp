/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
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
#include <QLibraryInfo>
#include <QTranslator>

#include "adldap.h"
#include "core/settings.h"
#include "core/admc_translator.h"

AdmcTranslator::AdmcTranslator() {
    translator = new QTranslator();
    adldap_translator = new QTranslator();
    qt_translator = new QTranslator();
    qtbase_translator = new QTranslator();
}

/**
 * Load the specified locale by installing new translators.
 *
 * @param locale Locale to load.
 * @return True on success, false on errors.
 */
bool AdmcTranslator::load_locale(QLocale locale) {
    const QString locale_dot_UTF8 = locale.name() + ".UTF-8";
    const char* locale_for_c =
        std::setlocale(LC_ALL, locale_dot_UTF8.toLocal8Bit().data());
    const QString translations_path =
        QLibraryInfo::path(QLibraryInfo::TranslationsPath);

    current_locale = locale;
    settings_set_variant(SETTING_locale, locale);

    QCoreApplication* app = QApplication::instance();

    if (! translator->isEmpty()) {
        QCoreApplication::removeTranslator(translator);
    }
    if (! adldap_translator->isEmpty()) {
        QCoreApplication::removeTranslator(adldap_translator);
    }
    if (! qt_translator->isEmpty()) {
        QCoreApplication::removeTranslator(qt_translator);
    }
    if (! qtbase_translator->isEmpty()) {
        QCoreApplication::removeTranslator(qtbase_translator);
    }

    bool result = true;
    bool load_result = false;

    if (! locale_for_c) {
        qDebug() << "Failed to set locale for C libs";
        result = false;
    }

    load_result = translator->load(locale, "admc", "_", ":/admc");
    app->installTranslator(translator);

    if (! load_result) {
        qDebug() << "Failed to load admc translation";
        result = false;
    }

    load_result = load_adldap_translation(*adldap_translator, locale);
    app->installTranslator(adldap_translator);

    if (! load_result) {
        qDebug() << "Failed to load adldap translation";
        result = false;
    }

    // NOTE: these translations are for qt-defined text, like standard dialog
    // buttons
    load_result = qt_translator->load(locale, "qt", "_", translations_path);
    app->installTranslator(qt_translator);

    if (! load_result) {
        qDebug() << "Failed to load qt translation";
        result = false;
    }

    load_result = qtbase_translator->load(locale, "qtbase", "_",
                                         translations_path);
    app->installTranslator(qtbase_translator);

    if (! load_result) {
        qDebug() << "Failed to load qt base translation";
        result = false;
    }

    return result;
}

/**
 * Load the locale saved in the application configuration.
 *
 * @return True on success, false on errors.
 */
bool AdmcTranslator::load_saved_locale() {
    const QLocale locale = settings_get_variant(SETTING_locale).toLocale();
    return load_locale(locale);
}

/**
 * Get the current application locale.
 */
QLocale AdmcTranslator::get_current_locale() {
    return current_locale;
}
