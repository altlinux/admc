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

/**
 * @file language.cpp
 * @brief Language menu action.
 */

#include <QLocale>

#include "core/settings.h"
#include "ui/action/language.h"

LanguageAction::LanguageAction(const QLocale &locale, QActionGroup *group)
    : QAction(locale.nativeLanguageName(), group)
{
    const QLocale CURRENT_LOCALE = settings_get_current_locale();
    // NOTE: Russian nativeLanguageName starts with lowercase letter for some
    // reason
    QString language_name = locale.nativeLanguageName();
    QChar first_letter_uppercased = language_name[0].toUpper();
    language_name.replace(0, 1, first_letter_uppercased);
    setText(language_name);
    setCheckable(true);
    setChecked(CURRENT_LOCALE.language() == locale.language());
}
