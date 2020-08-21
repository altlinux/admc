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

#include "ad_interface.h"

#include <QHash>

// TODO: I think scheme might have these mappings available BUT, it's unlikely that it has them in anything but English, soooo would prefer to not hardcode all of these here and use some external resource that already exists, BUT might be impossible due to needing a translation. Worst case scenario should at least bring this out into a resource file

// NOTE: have to do this like this because for translation to work, the "tr()" calls need to happen after QTranslator is setup
QString get_attribute_display_string(const QString &attribute) {
    static const QHash<QString, QString> display_strings = {
        {ATTRIBUTE_DISPLAY_NAME, QObject::tr("Full name")},
        {ATTRIBUTE_DESCRIPTION, QObject::tr("Description")},
        {ATTRIBUTE_USER_PRINCIPAL_NAME, QObject::tr("Logon name")},
        {ATTRIBUTE_SAMACCOUNT_NAME, QObject::tr("Logon name (pre-2000)")},
        {ATTRIBUTE_FIRST_NAME, QObject::tr("First name")},
        {ATTRIBUTE_LAST_NAME, QObject::tr("Last name")},
        {ATTRIBUTE_INITIALS, QObject::tr("Initials")},
        {ATTRIBUTE_DISTINGUISHED_NAME, QObject::tr("Canonical name")},
        {ATTRIBUTE_OBJECT_CLASS, QObject::tr("Object class")},
        {ATTRIBUTE_WHEN_CREATED, QObject::tr("Created")},
        {ATTRIBUTE_WHEN_CHANGED, QObject::tr("Changed")},
        {ATTRIBUTE_USN_CREATED, QObject::tr("USN created")},
        {ATTRIBUTE_USN_CHANGED, QObject::tr("USN changed")},
        {ATTRIBUTE_MAIL, QObject::tr("Email")},
        {ATTRIBUTE_OFFICE, QObject::tr("Office")},
        {ATTRIBUTE_TELEPHONE_NUMBER, QObject::tr("Phone")},
        {ATTRIBUTE_WWW_HOMEPAGE, QObject::tr("Homepage")},
    };
    static const QString default_value = QObject::tr("UNKNOWN ATTRIBUTE NAME");

    const QString display_string = display_strings.value(attribute, default_value);

    return display_string;
}
