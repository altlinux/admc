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

#include <QHash>
#include <QLocale>
#include <QString>

/**
 * A hash map of Qt locales to Windows language code identifier (MS-LCID)
 * values.
 *
 * See https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-lcid
 */
static const QHash<QLocale, int> LCID_MAP = {
    { QLocale::English, 409 },
    { QLocale::Russian, 419 },
};

/**
 * Convert a Qt locale to a Windows language code identifier (MS-LCID.)
 *
 * @param locale A locale to convert.
 * @return A language code identifier (LCID) for the specified locale, or LCID
 * for English locale if the given locale is not supported yet.
 */
int lcid_from_locale(const QLocale &locale) {
    return LCID_MAP.value(locale, LCID_MAP[QLocale::English]);
}

/**
 * Convert a Qt locale to a Windows language code identifier (MS-LCID) string.
 *
 * @see lcid_from_locale()
 * @param locale A locale to convert.
 * @return A language code identifier (LCID) for the specified locale as a
 * string, or LCID for English locale if the given locale is not supported yet.
 */
QString lcid_string_from_locale(const QLocale &locale) {
    return QString::number(lcid_from_locale(locale));
}
