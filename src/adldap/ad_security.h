/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2024 BaseALT Ltd.
 * Copyright (C) 2020-2024 Dmitry Degtyarev
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
#include <QLocale>

class AdInterface;
class AdConfig;
class AdObject;
struct security_descriptor;
struct dom_sid;
struct security_ace;
class CommonTaskManager;
typedef void TALLOC_CTX;

extern const QList<QString> well_known_sid_list;
extern const QList<uint32_t> common_rights_list;

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

// TODO: Unite SecutityRight and SecutityRightState structs into the ACE-like
// struct/class or remove them and use samba ACE structures. This and functions below
// can be implemented in security descriptor manager as an option.

struct SecurityRight {
    uint32_t access_mask;
    QByteArray object_type;
    QByteArray inherited_object_type;
    uint8_t flags;

    inline bool operator ==(const SecurityRight &another) const {
        return (another.access_mask == access_mask) &&
                (another.object_type == object_type) &&
                (another.inherited_object_type == inherited_object_type) &&
                (another.flags == flags);
    }
};
Q_DECLARE_METATYPE(SecurityRight)

extern CommonTaskManager *common_task_manager;

// ace_match_flags struct is used to configure ACE matching, that determines
// permissions check state and DACL's ACE search:
// - inherited ACEs match for all permissions with the same access mask;
// - object type match is required for correct ACE search and addition/deletion.
//
// SEC_ACE_FLAG_NO_PROPAGATE_INHERIT flag may be taken in account in future
struct ace_match_flags {
    bool match_inheritance;
    bool match_object_type;
};

QString ad_security_get_well_known_trustee_name(const QByteArray &trustee);
QString ad_security_get_trustee_name(AdInterface &ad, const QByteArray &trustee);
bool ad_security_get_protected_against_deletion(const AdObject &object);
bool ad_security_set_protected_against_deletion(AdInterface &ad, const QString dn, const bool enabled);
bool ad_security_get_user_cant_change_pass(const AdObject *object, AdConfig *adconfig);
bool ad_security_set_user_cant_change_pass(AdInterface *ad, const QString &dn, const bool enabled);
bool ad_security_replace_security_descriptor(AdInterface &ad, const QString &dn, security_descriptor *new_sd);
void ad_security_replace_dacl(security_descriptor *sd, const QList<security_ace> &new_dacl);

// Returns the full right name, adding "Write" or
// "Read" depending on access mask.
QString ad_security_get_right_name(AdConfig *adconfig, const SecurityRight &right, QLocale::Language language);

// NOTE: returned sd needs to be free'd with
// security_descriptor_free()
security_descriptor *security_descriptor_make_from_bytes(const QByteArray &sd_bytes);
security_descriptor *security_descriptor_make_from_bytes(TALLOC_CTX *mem_ctx, const QByteArray &sd_bytes);
security_descriptor *security_descriptor_copy(security_descriptor *sd);
void security_descriptor_free(security_descriptor *sd);
void security_descriptor_sort_dacl(security_descriptor *sd);
QList<QByteArray> security_descriptor_get_trustee_list(security_descriptor *sd);
SecurityRightState security_descriptor_get_right_state(const security_descriptor *sd, const QByteArray &trustee, const SecurityRight &right);
void security_descriptor_print(security_descriptor *sd, AdInterface &ad);
bool security_descriptor_verify_acl_order(security_descriptor *sd);
QList<security_ace> security_descriptor_get_dacl(const security_descriptor *sd);

// Remove all ACE's from DACL for given trustee. Note
// that inherited ACE's are untouched, so trustee might
// still have ace's remaining after this is called.
void security_descriptor_remove_trustee(security_descriptor *sd, const QList<QByteArray> &trustee_list);

// "Complete" versions of add/remove right f-ns that do A
// LOT more than just add rights. They also take care
// of superior and subordinate rights to follow a logic
// of a typical gui checklist of rights.
void security_descriptor_add_right(security_descriptor *sd, AdConfig *adconfig, const QList<QString> &class_list,
                                   const QByteArray &trustee, const SecurityRight &right, const bool allow);
void security_descriptor_remove_right(security_descriptor *sd, AdConfig *adconfig, const QList<QString> &class_list,
                                      const QByteArray &trustee, const SecurityRight &right, const bool allow);

QList<SecurityRight> ad_security_get_right_list_for_class(AdConfig *adconfig, const QList<QString> &class_list);
QList<SecurityRight> ad_security_get_common_rights();
QList<SecurityRight> ad_security_get_extended_rights_for_class(AdConfig *adconfig, const QList<QString> &class_list);
QList<SecurityRight> ad_security_get_superior_right_list(const SecurityRight &right);
QList<SecurityRight> ad_security_get_subordinate_right_list(AdConfig *adconfig, const SecurityRight &right, const QList<QString> &class_list);

QList<SecurityRight> creation_deletion_rights_for_class(AdConfig *adconfig, const QString &obj_class);
QList<SecurityRight> control_children_class_right(AdConfig *adconfig, const QString &obj_class);
QList<SecurityRight> children_class_read_write_prop_rights(AdConfig *adconfig, const QString &obj_class, const QString &attribute);
QList<SecurityRight> read_all_children_class_info_rights(AdConfig *adconfig, const QString &obj_class);
QList<SecurityRight> read_write_property_rights(AdConfig *adconfig, const QString &attribute);
QList<SecurityRight> create_children_class_right(AdConfig *adconfig, const QString &obj_class);

dom_sid dom_sid_from_bytes(const QByteArray &bytes);

#endif /* AD_SECURITY_H */
