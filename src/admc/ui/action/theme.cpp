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
 * @file theme.cpp
 * @brief Icon theme menu action.
 */

#include <QLocale>

#include "core/globals.h"
#include "core/managers/icon_manager.h"
#include "core/settings.h"
#include "ui/action/theme.h"

ThemeAction::ThemeAction(const QString &name, QActionGroup *group)
    : QAction(name, group)
{
    const QLocale CURRENT_LOCALE = settings_get_current_locale();
    const QString CURRENT_THEME = settings_get_current_icon_theme();
    const QString LOCALIZED_NAME =
        g_icon_manager->localized_theme_name(CURRENT_LOCALE, name);
    if (LOCALIZED_NAME.isEmpty()) {
        m_has_localized_name = false;
    } else {
        m_has_localized_name = true;
        setText(LOCALIZED_NAME);
    }

    setCheckable(true);
    setChecked(CURRENT_THEME == name);
}

/**
 * Check if the icon theme has a localized name.
 *
 * @return True if the icon theme has localized name, false otherwise.
 */
bool ThemeAction::has_localized_name() const {
    return m_has_localized_name;
}
