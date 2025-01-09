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

#ifndef AD_UTILS_H
#define AD_UTILS_H

/**
 * Various f-ns related to AD attributes which do not
 * interact with the server.
 */

#include "ad_defines.h"
#include <QHash>

class QString;
class QDateTime;
class QByteArray;
class AdConfig;
class QTranslator;
class QLocale;

bool large_integer_datetime_is_never(const QString &value);
QString datetime_qdatetime_to_string(const QString &attribute, const QDateTime &datetime, const AdConfig *adconfig);
QDateTime datetime_string_to_qdatetime(const QString &attribute, const QString &raw_value, const AdConfig *adconfig);

QString account_option_string(const AccountOption &option);
int account_option_bit(const AccountOption &option);

int group_scope_bit(GroupScope scope);
QString group_scope_string(GroupScope scope);

QString group_type_string(GroupType type);
QString group_type_string_adjective(GroupType type);

QString filesys_path_to_smb_path(const QString &sysvol_path);

QString extract_rid_from_sid(const QByteArray &sid, AdConfig *adconfig);

bool ad_string_to_bool(const QString &string);

QString dn_get_rdn(const QString &dn);
QString dn_get_name(const QString &dn);
QString dn_get_parent(const QString &dn);
QString dn_get_parent_canonical(const QString &dn);
QString dn_rename(const QString &dn, const QString &new_name);
QString dn_move(const QString &dn, const QString &new_parent_dn);
QString dn_canonical(const QString &dn);
QString dn_from_name_and_parent(const QString &name, const QString &parent, const QString &object_class);

QString get_default_domain_from_krb5();

int bitmask_set(const int input_mask, const int mask_to_set, const bool is_set);
bool bitmask_is_set(const int input_mask, const int mask_to_read);

// NOTE: uses a buffer that is capped at 100 strings, so
// pointers returned from this become invalid after 99 more
// calls. Only use this to give cstr args to C routines in
// the same scope. Keep this far away from any recursion.
const char *cstr(const QString &qstr);

// NOTE: you must call Q_INIT_RESOURCE(adldap) before
// calling this
bool load_adldap_translation(QTranslator &translator, const QLocale &locale);

QByteArray guid_string_to_bytes(const QString &guid_string);
QByteArray sid_string_to_bytes(const QString &sid_string);

QString attribute_type_display_string(const AttributeType type);

QString int_to_hex_string(const int n);

QHash<int, QString> attribute_value_bit_string_map(const QString &attribute);

QList<QString> bytearray_list_to_string_list(const QList<QByteArray> &bytearray_list);

#endif /* AD_UTILS_H */
