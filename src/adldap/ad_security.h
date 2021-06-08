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
#include <QString>
#include <QHash>
#include <QSet>

struct security_descriptor;
struct security_ace;
struct dom_sid;
class AdInterface;

extern const QHash<AcePermission, uint32_t> ace_permission_to_mask_map;
extern const QHash<AcePermission, QString> ace_permission_to_type_map;
extern const QList<AcePermission> all_permissions_list;
extern const QSet<AcePermission> all_permissions;
extern const QSet<AcePermission> access_permissions;
extern const QSet<AcePermission> read_prop_permissions;
extern const QSet<AcePermission> write_prop_permissions;

/**
 * Wrapper over "security_descriptor" struct mainly for the
 * purpose of automatically allocating and free'ing it's
 * memory. Also contains some access f-ns.
 */
class SecurityDescriptor {

public:
    SecurityDescriptor(const QByteArray &descriptor_bytes);
    ~SecurityDescriptor();

    QList<QByteArray> get_trustee_list() const;
    QList<security_ace *> dacl() const;
    void print_acl(const QByteArray &trustee) const;
    security_descriptor *get_data() const;

private:
    security_descriptor *data;
};

QByteArray dom_sid_to_bytes(const dom_sid &sid);
QString ad_security_get_trustee_name(AdInterface &ad, const QByteArray &trustee);
bool attribute_replace_security_descriptor(AdInterface &ad, const QString &dn, const QHash<QByteArray, QHash<AcePermission, PermissionState>> &descriptor_state_arg);

#endif /* AD_SECURITY_H */
