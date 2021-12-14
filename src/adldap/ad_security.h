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
 * Functions for working with AD security and security
 * descriptors.
 */

#ifndef AD_SECURITY_H
#define AD_SECURITY_H

#include "ad_defines.h"

#include <QByteArray>

class AdInterface;
class AdConfig;
class AdObject;
struct security_descriptor;
struct dom_sid;
typedef void TALLOC_CTX;

extern const QList<QString> well_known_sid_list;

enum SecurityRightStateType {
    SecurityRightStateType_Allow,
    SecurityRightStateType_Deny,
    SecurityRightStateType_COUNT,
};

enum SecurityRightStateInherited {
    SecurityRightStateInherited_Yes,
    SecurityRightStateInherited_No,
    SecurityRightStateInherited_COUNT,
}; 

class SecurityRightState {
public:
    SecurityRightState(const bool data[SecurityRightStateInherited_COUNT][SecurityRightStateType_COUNT]);

    bool get(const SecurityRightStateInherited inherited, const SecurityRightStateType type) const;

private:
    bool data[SecurityRightStateInherited_COUNT][SecurityRightStateType_COUNT];
};

QString ad_security_get_well_known_trustee_name(const QByteArray &trustee);
QString ad_security_get_trustee_name(AdInterface &ad, const QByteArray &trustee);
bool ad_security_get_protected_against_deletion(const AdObject &object);
bool ad_security_set_protected_against_deletion(AdInterface &ad, const QString dn, const bool enabled);
bool ad_security_get_user_cant_change_pass(const AdObject *object, AdConfig *adconfig);
bool ad_security_set_user_cant_change_pass(AdInterface *ad, const QString &dn, const bool enabled);
bool ad_security_replace_security_descriptor(AdInterface &ad, const QString &dn, security_descriptor *new_sd);

// Returns the full right name, adding "Write" or
// "Read" depending on access mask.
QString ad_security_get_right_name(AdConfig *adconfig, const uint32_t access_mask, const QByteArray &object_type);

// NOTE: returned sd needs to be free'd with
// security_descriptor_free()
security_descriptor *security_descriptor_make_from_bytes(const QByteArray &sd_bytes);
security_descriptor *security_descriptor_make_from_bytes(TALLOC_CTX *mem_ctx, const QByteArray &sd_bytes);
security_descriptor *security_descriptor_copy(security_descriptor *sd);
void security_descriptor_free(security_descriptor *sd);
void security_descriptor_sort_dacl(security_descriptor *sd);
QList<QByteArray> security_descriptor_get_trustee_list(security_descriptor *sd);
SecurityRightState security_descriptor_get_right(const security_descriptor *sd, const QByteArray &trustee, const uint32_t access_mask, const QByteArray &object_type);

// These f-ns do only the requested operation. For
// example, if some right is currently denied and you
// want to allow it, just calling add_right() is not
// enough. You would need to remove the "Deny" entry
// for this right using remove_right() and add "Allow"
// using add_right(). In addition, these f-ns only edit
// object rights and don't touch inherited rights. Note
// that order of operations matters, a right won't be
// added if it's already applied by a generic right
// that contains it. In that case you'd have to first
// remove the generic right and then add the specific
// right.
void security_descriptor_add_right(security_descriptor *sd, const QByteArray &trustee, const uint32_t access_mask, const QByteArray &object_type, const bool allow);
void security_descriptor_remove_right(security_descriptor *sd, const QByteArray &trustee, const uint32_t access_mask, const QByteArray &object_type, const bool allow);

// Remove all ACE's from DACL for given trustee. Note
// that inherited ACE's are untouched, so trustee might
// still have ace's remaining after this is called.
void security_descriptor_remove_trustee(security_descriptor *sd, const QList<QByteArray> &trustee_list);

#endif /* AD_SECURITY_H */
