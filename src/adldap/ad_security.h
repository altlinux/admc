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

/** 
 * Functions for working with security descriptors.
 */

#ifndef AD_SECURITY_H
#define AD_SECURITY_H

#include "ad_defines.h"

#include <QByteArray>
#include <QHash>
#include <QSet>
#include <QString>

class AdInterface;
class AdConfig;
class AdObject;
struct security_descriptor;
struct dom_sid;

extern const QList<QString> well_known_sid_list;
extern const QHash<AcePermission, uint32_t> ace_permission_to_mask_map;
extern const QHash<AcePermission, QString> ace_permission_to_type_map;
extern const QList<AcePermission> all_permissions_list;
extern const QSet<AcePermission> all_permissions;
extern const QSet<AcePermission> access_permissions;
extern const QSet<AcePermission> read_prop_permissions;
extern const QSet<AcePermission> write_prop_permissions;

QHash<QByteArray, QHash<AcePermission, PermissionState>> ad_security_modify(const QHash<QByteArray, QHash<AcePermission, PermissionState>> &current, const QByteArray &trustee, const AcePermission permission, const PermissionState new_state);
QString ad_security_get_well_known_trustee_name(const QByteArray &trustee);
QString ad_security_get_trustee_name(AdInterface &ad, const QByteArray &trustee);
bool attribute_replace_security_descriptor(AdInterface *ad, const QString &dn, const QHash<QByteArray, QHash<AcePermission, PermissionState>> &descriptor_state_arg);
QList<QByteArray> ad_security_get_trustee_list_from_object(const AdObject &object);
QHash<QByteArray, QHash<AcePermission, PermissionState>> ad_security_get_state_from_sd(security_descriptor *sd, AdConfig *adconfig);

// NOTE: have to talloc_free() returned sd
security_descriptor *ad_security_get_sd(const AdObject &object);

void ad_security_sort_dacl(security_descriptor *sd);

QByteArray dom_sid_to_bytes(const dom_sid &sid);

#endif /* AD_SECURITY_H */
