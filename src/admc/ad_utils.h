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

#ifndef AD_UTILS_H
#define AD_UTILS_H

/**
 * Varioues f-ns related to AD attributes which do not
 * interact with the server.
 */

#include "ad_defines.h"

#include <QString>
#include <QDateTime>
#include <QByteArray>

bool large_integer_datetime_is_never(const QString &value);
QString datetime_qdatetime_to_string(const QString &attribute, const QDateTime &datetime);
QDateTime datetime_string_to_qdatetime(const QString &attribute, const QString &raw_value);

QString account_option_string(const AccountOption &option);
int account_option_bit(const AccountOption &option);

int group_scope_bit(GroupScope scope);
QString group_scope_string(GroupScope scope);

QString group_type_string(GroupType type);

QString sysvol_path_to_smb(const QString &sysvol_path);

QString extract_rid_from_sid(const QByteArray &sid);

QString dn_get_name(const QString &dn);
QString dn_get_parent_canonical(const QString &dn);
QString dn_rename(const QString &dn, const QString &new_name);
QString dn_canonical(const QString &dn);

#endif /* AD_UTILS_H */
