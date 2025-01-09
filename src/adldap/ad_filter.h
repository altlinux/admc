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

#ifndef AD_FILTER_H
#define AD_FILTER_H

/**
 * Functions for constructing an LDAP filter.
 */

#include <QString>

enum Condition {
    Condition_Contains,
    Condition_Equals,
    Condition_NotEquals,
    Condition_StartsWith,
    Condition_EndsWith,
    Condition_Set,
    Condition_Unset,
    Condition_COUNT,
};

extern const QList<QString> filter_classes;

QString filter_CONDITION(const Condition condition, const QString &attribute, const QString &value = QString());

// If arguments for AND/OR are empty, empty string is returned
QString filter_AND(const QList<QString> &subfilters);
QString filter_OR(const QList<QString> &subfilters);

// Adds advanced view filter, depending on current advanced
// view setting
QString add_advanced_view_filter(const QString &filter);

QString condition_to_display_string(const Condition condition);

// Search all the way to the root through the chain of
// ancestry until it finds a match. This method works only with DN-type
// attributes
QString filter_matching_rule_in_chain(const QString &attribute, const QString &dn_value);

// Filter that accepts any DN from given list
QString filter_dn_list(const QList<QString> &dn_list);

#endif /* AD_FILTER_H */
