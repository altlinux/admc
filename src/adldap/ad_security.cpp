/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "ad_security.h"

#include "adldap.h"

#include "samba/dom_sid.h"
#include "samba/libsmb_xattr.h"
#include "samba/ndr_security.h"
#include "samba/security_descriptor.h"

#include "ad_filter.h"

#include <QDebug>

#define UNUSED_ARG(x) (void) (x)

QByteArray dom_sid_to_bytes(const dom_sid &sid);
dom_sid dom_sid_from_bytes(const QByteArray &bytes);
QByteArray dom_sid_string_to_bytes(const dom_sid &sid);
bool check_ace_match(const security_ace &ace, const QByteArray &trustee, const QByteArray &object_type, const bool allow, const bool inherited);
QList<security_ace> security_descriptor_get_dacl(const security_descriptor *sd);
void ad_security_replace_dacl(security_descriptor *sd, const QList<security_ace> &new_dacl);
uint32_t ad_security_map_access_mask(const uint32_t access_mask);
int ace_compare_simplified(const security_ace &ace1, const security_ace &ace2);

// NOTE: these "base" f-ns are used by the full
// versions of add/remove right f-ns. Base f-ns do only
// the bare minimum, just adding/removing matching ace,
// without handling opposites, subordinates, superiors,
// etc. They also don't sort ACL, callers need to
// handle sorting themselves.
void security_descriptor_add_right_base(security_descriptor *sd, const QByteArray &trustee, const uint32_t access_mask, const QByteArray &object_type, const bool allow);
void security_descriptor_remove_right_base(security_descriptor *sd, const QByteArray &trustee, const uint32_t access_mask, const QByteArray &object_type, const bool allow);

const QList<int> ace_types_with_object = {
    SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT,
    SEC_ACE_TYPE_ACCESS_DENIED_OBJECT,
    SEC_ACE_TYPE_SYSTEM_AUDIT_OBJECT,
    SEC_ACE_TYPE_SYSTEM_ALARM_OBJECT,
};

const QList<QString> well_known_sid_list = {
    SID_WORLD_DOMAIN,
    SID_WORLD,
    SID_WORLD,
    SID_CREATOR_OWNER_DOMAIN,
    SID_CREATOR_OWNER,
    SID_CREATOR_GROUP,
    SID_OWNER_RIGHTS,
    SID_NT_AUTHORITY,
    SID_NT_DIALUP,
    SID_NT_NETWORK,
    SID_NT_BATCH,
    SID_NT_INTERACTIVE,
    SID_NT_SERVICE,
    SID_NT_ANONYMOUS,
    SID_NT_PROXY,
    SID_NT_ENTERPRISE_DCS,
    SID_NT_SELF,
    SID_NT_AUTHENTICATED_USERS,
    SID_NT_RESTRICTED,
    SID_NT_TERMINAL_SERVER_USERS,
    SID_NT_REMOTE_INTERACTIVE,
    SID_NT_THIS_ORGANISATION,
    SID_NT_IUSR,
    SID_NT_SYSTEM,
    SID_NT_LOCAL_SERVICE,
    SID_NT_NETWORK_SERVICE,
    SID_NT_DIGEST_AUTHENTICATION,
    SID_NT_NTLM_AUTHENTICATION,
    SID_NT_SCHANNEL_AUTHENTICATION,
    SID_NT_OTHER_ORGANISATION,
};

const QHash<QString, QString> trustee_name_map = {
    {SID_WORLD_DOMAIN, "Everyone in Domain"},
    {SID_WORLD, "Everyone"},
    {SID_CREATOR_OWNER_DOMAIN, "CREATOR OWNER DOMAIN"},
    {SID_CREATOR_OWNER, "CREATOR OWNER"},
    {SID_CREATOR_GROUP, "CREATOR GROUP"},
    {SID_OWNER_RIGHTS, "OWNER RIGHTS"},
    {SID_NT_AUTHORITY, "AUTHORITY"},
    {SID_NT_DIALUP, "DIALUP"},
    {SID_NT_NETWORK, "NETWORK"},
    {SID_NT_BATCH, "BATCH"},
    {SID_NT_INTERACTIVE, "INTERACTIVE"},
    {SID_NT_SERVICE, "SERVICE"},
    {SID_NT_ANONYMOUS, "ANONYMOUS LOGON"},
    {SID_NT_PROXY, "PROXY"},
    {SID_NT_ENTERPRISE_DCS, "ENTERPRISE DOMAIN CONTROLLERS"},
    {SID_NT_SELF, "SELF"},
    {SID_NT_AUTHENTICATED_USERS, "Authenticated Users"},
    {SID_NT_RESTRICTED, "RESTRICTED"},
    {SID_NT_TERMINAL_SERVER_USERS, "TERMINAL SERVER USERS"},
    {SID_NT_REMOTE_INTERACTIVE, "REMOTE INTERACTIVE LOGON"},
    {SID_NT_THIS_ORGANISATION, "This Organization"},
    {SID_NT_IUSR, "IUSR"},
    {SID_NT_SYSTEM, "SYSTEM"},
    {SID_NT_LOCAL_SERVICE, "LOCAL SERVICE"},
    {SID_NT_NETWORK_SERVICE, "NETWORK SERVICE"},
    {SID_NT_DIGEST_AUTHENTICATION, "Digest Authentication"},
    {SID_NT_NTLM_AUTHENTICATION, "NTLM Authentication"},
    {SID_NT_SCHANNEL_AUTHENTICATION, "SChannel Authentication"},
    {SID_NT_OTHER_ORGANISATION, "Other Organization"},
};

const QList<QString> cant_change_pass_trustee_cn_list = {
    SID_NT_SELF,
    SID_WORLD,
};

const QList<uint32_t> protect_deletion_mask_list = {
    SEC_STD_DELETE,
    SEC_ADS_DELETE_TREE,
};

const QSet<security_ace_type> ace_type_allow_set = {
    SEC_ACE_TYPE_ACCESS_ALLOWED,
    SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT,
};
const QSet<security_ace_type> ace_type_deny_set = {
    SEC_ACE_TYPE_ACCESS_DENIED,
    SEC_ACE_TYPE_ACCESS_DENIED_OBJECT,
};

// NOTE: this is also used for display order
const QList<uint32_t> common_rights_list = {
    SEC_ADS_GENERIC_ALL,
    SEC_ADS_GENERIC_READ,
    SEC_ADS_GENERIC_WRITE,
    SEC_ADS_CREATE_CHILD,
    SEC_ADS_DELETE_CHILD,
};

// NOTE: This is needed because for some reason,
// security editing in RSAT has a different value for
// generic read, without the "list object" right. Need
// to remove that bit both when setting generic read
// and when reading it.
#define GENERIC_READ_FIXED (SEC_ADS_GENERIC_READ & ~SEC_ADS_LIST_OBJECT)

SecurityRightState::SecurityRightState(const bool data_arg[SecurityRightStateInherited_COUNT][SecurityRightStateType_COUNT]) {
    for (int inherited = 0; inherited < SecurityRightStateInherited_COUNT; inherited++) {
        for (int type = 0; type < SecurityRightStateType_COUNT; type++) {
            data[inherited][type] = data_arg[inherited][type];
        }
    }
}

bool SecurityRightState::get(const SecurityRightStateInherited inherited, const SecurityRightStateType type) const {
    return data[inherited][type];
}

security_descriptor *security_descriptor_make_from_bytes(TALLOC_CTX *mem_ctx, const QByteArray &sd_bytes) {
    DATA_BLOB blob = data_blob_const(sd_bytes.data(), sd_bytes.size());

    security_descriptor *out = talloc(mem_ctx, struct security_descriptor);

    ndr_pull_struct_blob(&blob, out, out, (ndr_pull_flags_fn_t) ndr_pull_security_descriptor);

    return out;
}

security_descriptor *security_descriptor_make_from_bytes(const QByteArray &sd_bytes) {
    security_descriptor *out = security_descriptor_make_from_bytes(NULL, sd_bytes);

    return out;
}

void security_descriptor_free(security_descriptor *sd) {
    talloc_free(sd);
}

security_descriptor *security_descriptor_copy(security_descriptor *sd) {
    security_descriptor *out = talloc(NULL, struct security_descriptor);

    out = security_descriptor_copy(out, sd);

    return out;
}

QString ad_security_get_well_known_trustee_name(const QByteArray &trustee) {
    const QString trustee_string = object_sid_display_value(trustee);
    return trustee_name_map.value(trustee_string, QString());
}

QString ad_security_get_trustee_name(AdInterface &ad, const QByteArray &trustee) {
    const QString trustee_string = object_sid_display_value(trustee);

    if (trustee_name_map.contains(trustee_string)) {
        return trustee_name_map[trustee_string];
    } else {
        // Try to get name of trustee by finding it's DN
        const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_SID, trustee_string);
        const QList<QString> attributes = {
            ATTRIBUTE_DISPLAY_NAME,
            ATTRIBUTE_SAM_ACCOUNT_NAME,
        };
        const auto trustee_search = ad.search(ad.adconfig()->domain_dn(), SearchScope_All, filter, QList<QString>());
        if (!trustee_search.isEmpty()) {
            // NOTE: this is some weird name selection logic
            // but that's how microsoft does it. Maybe need
            // to use this somewhere else as well?
            const QString name = [&]() {
                const AdObject object = trustee_search.values()[0];

                if (object.contains(ATTRIBUTE_DISPLAY_NAME)) {
                    return object.get_string(ATTRIBUTE_DISPLAY_NAME);
                } else if (object.contains(ATTRIBUTE_SAM_ACCOUNT_NAME)) {
                    return object.get_string(ATTRIBUTE_SAM_ACCOUNT_NAME);
                } else {
                    return dn_get_name(object.get_dn());
                }
            }();

            return name;
        } else {
            // Return raw sid as last option
            return trustee_string;
        }
    }
}

bool ad_security_replace_security_descriptor(AdInterface &ad, const QString &dn, security_descriptor *new_sd) {
    const QByteArray new_descriptor_bytes = [&]() {
        TALLOC_CTX *tmp_ctx = talloc_new(NULL);

        DATA_BLOB blob;
        ndr_push_struct_blob(&blob, tmp_ctx, new_sd, (ndr_push_flags_fn_t) ndr_push_security_descriptor);

        const QByteArray out = QByteArray((char *) blob.data, blob.length);

        talloc_free(tmp_ctx);

        return out;
    }();

    const bool set_dacl = true;
    const bool apply_success = ad.attribute_replace_value(dn, ATTRIBUTE_SECURITY_DESCRIPTOR, new_descriptor_bytes, DoStatusMsg_Yes, set_dacl);

    return apply_success;
}

QByteArray dom_sid_to_bytes(const dom_sid &sid) {
    const QByteArray bytes = QByteArray((char *) &sid, sizeof(struct dom_sid));

    return bytes;
}

// Copy sid bytes into dom_sid struct and adds padding
// if necessary
dom_sid dom_sid_from_bytes(const QByteArray &bytes) {
    dom_sid out;
    memset(&out, '\0', sizeof(dom_sid));
    memcpy(&out, bytes.data(), sizeof(dom_sid));

    return out;
}

QByteArray dom_sid_string_to_bytes(const QString &string) {
    dom_sid sid;
    dom_sid_parse(cstr(string), &sid);
    const QByteArray bytes = dom_sid_to_bytes(sid);

    return bytes;
}

void security_descriptor_sort_dacl(security_descriptor *sd) {
    qsort(sd->dacl->aces, sd->dacl->num_aces, sizeof(security_ace), ace_compare);
}

bool ad_security_get_protected_against_deletion(const AdObject &object) {
    security_descriptor *sd = object.get_security_descriptor();

    const QByteArray trustee_everyone = sid_string_to_bytes(SID_WORLD);

    const bool is_enabled_for_trustee = [&]() {
        for (const uint32_t &mask : protect_deletion_mask_list) {
            const SecurityRightState state = security_descriptor_get_right(sd, trustee_everyone, mask, QByteArray());

            const bool deny = state.get(SecurityRightStateInherited_No, SecurityRightStateType_Deny);

            if (!deny) {
                return false;
            }
        }

        return true;
    }();

    security_descriptor_free(sd);

    return is_enabled_for_trustee;
}

bool ad_security_get_user_cant_change_pass(const AdObject *object, AdConfig *adconfig) {
    security_descriptor *sd = object->get_security_descriptor();

    const bool enabled = [&]() {
        bool out = false;

        for (const QString &trustee_cn : cant_change_pass_trustee_cn_list) {
            const bool is_denied = [&]() {
                const QByteArray trustee = sid_string_to_bytes(trustee_cn);
                const QByteArray change_pass_right = adconfig->get_right_guid("User-Change-Password");
                const SecurityRightState state = security_descriptor_get_right(sd, trustee, SEC_ADS_CONTROL_ACCESS, change_pass_right);
                const bool out_denied = state.get(SecurityRightStateInherited_No, SecurityRightStateType_Deny);

                return out_denied;
            }();

            // Enabled if enabled for either of the
            // trustee's. Both don't have to be
            // enabled
            if (is_denied) {
                out = true;

                break;
            }
        }

        return out;
    }();

    security_descriptor_free(sd);

    return enabled;
}

bool ad_security_set_user_cant_change_pass(AdInterface *ad, const QString &dn, const bool enabled) {
    security_descriptor *sd = [&]() {
        const AdObject object = ad->search_object(dn, {ATTRIBUTE_SECURITY_DESCRIPTOR});
        security_descriptor *out = object.get_security_descriptor();

        return out;
    }();

    for (const QString &trustee_cn : cant_change_pass_trustee_cn_list) {
        const QByteArray trustee = sid_string_to_bytes(trustee_cn);
        const QByteArray change_pass_right = ad->adconfig()->get_right_guid("User-Change-Password");

        // NOTE: the logic is a bit confusing here with
        // all the layers of negation but: "enabled"
        // means "denied", so we remove the opposite of
        // what we want, and add the type of right that
        // we want
        // NOTE: using "base" f-ns because we don't want
        // to touch superiors/subordinates
        const bool allow = !enabled;
        security_descriptor_remove_right_base(sd, trustee, SEC_ADS_CONTROL_ACCESS, change_pass_right, !allow);
        security_descriptor_add_right_base(sd, trustee, SEC_ADS_CONTROL_ACCESS, change_pass_right, allow);
    }

    security_descriptor_sort_dacl(sd);

    const bool success = ad_security_replace_security_descriptor(*ad, dn, sd);

    security_descriptor_free(sd);

    return success;
}

bool ad_security_set_protected_against_deletion(AdInterface &ad, const QString dn, const bool enabled) {
    const AdObject object = ad.search_object(dn);

    const bool is_enabled = ad_security_get_protected_against_deletion(object);

    const bool dont_need_to_change = (is_enabled == enabled);
    if (dont_need_to_change) {
        return true;
    }

    security_descriptor *new_sd = [&]() {
        security_descriptor *out = object.get_security_descriptor();

        const QByteArray trustee_everyone = sid_string_to_bytes(SID_WORLD);

        // NOTE: we only add/remove deny entries. If
        // there are any allow entries, they are
        // untouched.
        for (const uint32_t &mask : protect_deletion_mask_list) {
            if (enabled) {
                security_descriptor_add_right_base(out, trustee_everyone, mask, QByteArray(), false);
            } else {
                security_descriptor_remove_right_base(out, trustee_everyone, mask, QByteArray(), false);
            }
        }

        return out;
    }();

    security_descriptor_sort_dacl(new_sd);

    const bool apply_success = ad_security_replace_security_descriptor(ad, dn, new_sd);

    security_descriptor_free(new_sd);

    return apply_success;
}

QList<QByteArray> security_descriptor_get_trustee_list(security_descriptor *sd) {
    const QSet<QByteArray> trustee_set = [&]() {
        QSet<QByteArray> out;

        const QList<security_ace> dacl = security_descriptor_get_dacl(sd);

        for (const security_ace &ace : dacl) {
            const QByteArray trustee = dom_sid_to_bytes(ace.trustee);

            out.insert(trustee);
        }

        return out;
    }();

    const QList<QByteArray> trustee_list = QList<QByteArray>(trustee_set.begin(), trustee_set.end());

    return trustee_list;
}

QList<security_ace> security_descriptor_get_dacl(const security_descriptor *sd) {
    QList<security_ace> out;

    security_acl *dacl = sd->dacl;

    for (size_t i = 0; i < dacl->num_aces; i++) {
        security_ace ace = dacl->aces[i];

        out.append(ace);
    }

    return out;
}

SecurityRightState security_descriptor_get_right(const security_descriptor *sd, const QByteArray &trustee, const uint32_t access_mask_arg, const QByteArray &object_type) {
    bool out_data[SecurityRightStateInherited_COUNT][SecurityRightStateType_COUNT];

    const uint32_t access_mask = ad_security_map_access_mask(access_mask_arg);

    for (int x = 0; x < SecurityRightStateInherited_COUNT; x++) {
        for (int y = 0; y < SecurityRightStateType_COUNT; y++) {
            out_data[x][y] = false;
        }
    }

    const QList<security_ace> dacl = security_descriptor_get_dacl(sd);

    for (const security_ace &ace : dacl) {
        const bool match = [&]() {
            const bool trustee_match = [&]() {
                const dom_sid trustee_sid = dom_sid_from_bytes(trustee);
                const bool trustees_are_equal = (dom_sid_compare(&ace.trustee, &trustee_sid) == 0);

                return trustees_are_equal;
            }();

            const bool access_mask_match = bitmask_is_set(ace.access_mask, access_mask);

            const bool object_match = [&]() {
                const bool object_present = ace_types_with_object.contains(ace.type);

                if (object_present) {
                    const GUID out_guid = ace.object.object.type.type;
                    const QByteArray ace_object_type = QByteArray((char *) &out_guid, sizeof(GUID));
                    const bool types_are_equal = (ace_object_type == object_type);

                    return types_are_equal;
                } else {
                    // NOTE: if compared ace doesn't
                    // have an object it can still
                    // match if it's access mask
                    // matches with given ace. Example:
                    // ace that allows "generic read"
                    // (mask contains bit for "read
                    // property" and object is empty)
                    // will also allow right for
                    // reading personal info (mask *is*
                    // "read property" and contains
                    // some object)
                    return access_mask_match;
                }
            }();

            const bool out = (trustee_match && access_mask_match && object_match);

            return out;
        }();

        if (match) {
            bool state_list[2];
            state_list[SecurityRightStateType_Allow] = ace_type_allow_set.contains(ace.type);
            state_list[SecurityRightStateType_Deny] = ace_type_deny_set.contains(ace.type);

            const int inherit_i = [&]() {
                const bool ace_is_inherited = bitmask_is_set(ace.flags, SEC_ACE_FLAG_INHERITED_ACE);

                if (ace_is_inherited) {
                    return SecurityRightStateInherited_Yes;
                } else {
                    return SecurityRightStateInherited_No;
                }
            }();

            for (int type_i = 0; type_i < SecurityRightStateType_COUNT; type_i++) {
                const bool right_state = state_list[type_i];

                if (right_state) {
                    out_data[inherit_i][type_i] = true;
                }
            }
        }
    }

    const SecurityRightState out = SecurityRightState(out_data);

    return out;
}

void security_descriptor_print(security_descriptor *sd, AdInterface &ad) {
    const QList<security_ace> dacl = security_descriptor_get_dacl(sd);

    for (const security_ace &ace : dacl) {
        qInfo() << "\nace:";
        const QByteArray trustee_sid = dom_sid_to_bytes(ace.trustee);
        const QString trustee_name = ad_security_get_trustee_name(ad, trustee_sid);
        qInfo() << "trustee:" << trustee_name;
        qInfo() << "mask:" << int_to_hex_string(ace.access_mask);
        qInfo() << "type:" << ace.type;
    }
}

void security_descriptor_add_right_base(security_descriptor *sd, const QByteArray &trustee, const uint32_t access_mask_arg, const QByteArray &object_type, const bool allow) {
    const uint32_t access_mask = ad_security_map_access_mask(access_mask_arg);

    const QList<security_ace> dacl = security_descriptor_get_dacl(sd);

    const int matching_index = [&]() {
        for (int i = 0; i < dacl.size(); i++) {
            const security_ace ace = dacl[i];

            // NOTE: access mask match doesn't matter
            // because we also want to add right to
            // existing ace, if it exists. In that case
            // such ace would not match by mask and
            // that's fine.
            const bool match = check_ace_match(ace, trustee, object_type, allow, false);

            if (match) {
                return i;
            }
        }

        return -1;
    }();

    if (matching_index != -1) {
        const bool right_already_set = [&]() {
            const security_ace matching_ace = dacl[matching_index];
            const bool out = bitmask_is_set(matching_ace.access_mask, access_mask);

            return out;
        }();

        // Matching ace exists, so reuse it by adding
        // given mask to this ace, but only if it's not set already
        if (!right_already_set) {
            security_ace new_ace = dacl[matching_index];
            new_ace.access_mask = bitmask_set(new_ace.access_mask, access_mask, true);
            sd->dacl->aces[matching_index] = new_ace;
        }
    } else {
        // No matching ace, so make a new ace for this
        // right
        const security_ace ace = [&]() {
            security_ace out;

            const bool object_present = !object_type.isEmpty();

            out.type = [&]() {
                if (allow) {
                    if (object_present) {
                        return SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT;
                    } else {
                        return SEC_ACE_TYPE_ACCESS_ALLOWED;
                    }
                } else {
                    if (object_present) {
                        return SEC_ACE_TYPE_ACCESS_DENIED_OBJECT;
                    } else {
                        return SEC_ACE_TYPE_ACCESS_DENIED;
                    }
                }

                return SEC_ACE_TYPE_ACCESS_ALLOWED;
            }();

            out.flags = 0x00;
            out.access_mask = access_mask;
            out.object.object.flags = [&]() {
                if (object_present) {
                    return SEC_ACE_OBJECT_TYPE_PRESENT;
                } else {
                    return 0;
                }
            }();

            if (object_present) {
                out.object.object.type.type = [&]() {
                    struct GUID type_guid;
                    memcpy(&type_guid, object_type.data(), sizeof(GUID));

                    return type_guid;
                }();
            }

            out.trustee = dom_sid_from_bytes(trustee);

            return out;
        }();

        security_descriptor_dacl_add(sd, &ace);
    }
}

// Checks if ace matches given members. Note that
// access masks are not compared. Compare them yourself
// if you need to further filter by masks.
bool check_ace_match(const security_ace &ace, const QByteArray &trustee, const QByteArray &object_type, const bool allow, const bool inherited) {
    const bool type_match = [&]() {
        const security_ace_type ace_type = ace.type;

        const bool ace_allow = ace_type_allow_set.contains(ace_type);
        const bool ace_deny = ace_type_deny_set.contains(ace_type);

        if (allow && ace_allow) {
            return true;
        } else if (!allow && ace_deny) {
            return true;
        } else {
            return false;
        }
    }();

    const bool flags_match = [&]() {
        const bool ace_is_inherited = bitmask_is_set(ace.flags, SEC_ACE_FLAG_INHERITED_ACE);
        const bool out = (ace_is_inherited == inherited);

        return out;
    }();

    const bool trustee_match = [&]() {
        const dom_sid trustee_sid = dom_sid_from_bytes(trustee);
        const bool trustees_are_equal = (dom_sid_compare(&ace.trustee, &trustee_sid) == 0);

        return trustees_are_equal;
    }();

    const bool object_match = [&]() {
        const bool object_present = ace_types_with_object.contains(ace.type);

        if (object_present) {
            const GUID ace_object_type_guid = ace.object.object.type.type;
            const QByteArray ace_object_type = QByteArray((char *) &ace_object_type_guid, sizeof(GUID));
            const bool types_are_equal = (ace_object_type == object_type);

            return types_are_equal;
        } else {
            return object_type.isEmpty();
        }
    }();

    const bool out_match = (type_match && flags_match && trustee_match && object_match);

    return out_match;
}

void security_descriptor_remove_right_base(security_descriptor *sd, const QByteArray &trustee, const uint32_t access_mask_arg, const QByteArray &object_type, const bool allow) {
    const uint32_t access_mask = ad_security_map_access_mask(access_mask_arg);

    const QList<security_ace> new_dacl = [&]() {
        QList<security_ace> out;

        const QList<security_ace> old_dacl = security_descriptor_get_dacl(sd);

        for (const security_ace &ace : old_dacl) {
            const bool match = check_ace_match(ace, trustee, object_type, allow, false);
            const bool ace_mask_contains_mask = bitmask_is_set(ace.access_mask, access_mask);

            if (match && ace_mask_contains_mask) {
                const security_ace edited_ace = [&]() {
                    security_ace out_ace = ace;

                    // NOTE: need to handle a special
                    // case due to read and write
                    // rights sharing the "read
                    // control" bit. When setting
                    // either read/write, don't change
                    // that shared bit if the other of
                    // these rights is set
                    const uint32_t mask_to_unset = [&]() {
                        const QHash<uint32_t, uint32_t> opposite_map = {
                            {GENERIC_READ_FIXED, SEC_ADS_GENERIC_WRITE},
                            {SEC_ADS_GENERIC_WRITE, GENERIC_READ_FIXED},
                        };

                        if (opposite_map.contains(access_mask)) {
                            const uint32_t opposite = opposite_map[access_mask];
                            const bool opposite_is_set = bitmask_is_set(ace.access_mask, opposite);

                            if (opposite_is_set) {
                                const uint32_t out_mask = (access_mask & ~SEC_STD_READ_CONTROL);

                                return out_mask;
                            } else {
                                return access_mask;
                            }
                        } else {
                            return access_mask;
                        }
                    }();

                    out_ace.access_mask = bitmask_set(ace.access_mask, mask_to_unset, false);

                    return out_ace;
                }();

                const bool edited_ace_became_empty = (edited_ace.access_mask == 0);

                if (!edited_ace_became_empty) {
                    out.append(edited_ace);
                }
            } else {
                out.append(ace);
            }
        }

        return out;
    }();

    ad_security_replace_dacl(sd, new_dacl);
}

void security_descriptor_remove_trustee(security_descriptor *sd, const QList<QByteArray> &trustee_list) {
    const QList<security_ace> new_dacl = [&]() {
        QList<security_ace> out;

        const QList<security_ace> old_dacl = security_descriptor_get_dacl(sd);

        for (const security_ace &ace : old_dacl) {
            const bool match = [&]() {
                const bool trustee_match = [&]() {
                    for (const QByteArray &trustee : trustee_list) {
                        const dom_sid trustee_sid = dom_sid_from_bytes(trustee);
                        const bool trustees_are_equal = (dom_sid_compare(&ace.trustee, &trustee_sid) == 0);

                        if (trustees_are_equal) {
                            return true;
                        }
                    }

                    return false;
                }();

                const bool inherited = bitmask_is_set(ace.flags, SEC_ACE_FLAG_INHERITED_ACE);

                const bool out_match = trustee_match && !inherited;

                return out_match;
            }();

            if (!match) {
                out.append(ace);
            }
        }

        return out;
    }();

    ad_security_replace_dacl(sd, new_dacl);
}

// TODO: Need to verify SACL order as well, because
// advanced security dialog(to be implemented) edits
// SACL.
bool security_descriptor_verify_acl_order(security_descriptor *sd) {
    security_descriptor *copy = security_descriptor_copy(sd);

    const bool order_is_correct = [&]() {
        bool out = true;

        QList<security_ace> dacl = security_descriptor_get_dacl(copy);

        security_ace curr = dacl.takeFirst();

        while (!dacl.isEmpty()) {
            security_ace next = dacl.takeFirst();
            const int comparison = ace_compare_simplified(curr, next);
            const bool order_is_good = (comparison <= 0);

            if (!order_is_good) {
                out = false;
            }

            curr = next;
        }

        return out;
    }();

    security_descriptor_free(copy);

    return order_is_correct;
}

QString ad_security_get_right_name(AdConfig *adconfig, const uint32_t access_mask, const QByteArray &object_type, const QLocale::Language language) {
    const QString object_type_name = adconfig->get_right_name(object_type, language);

    if (access_mask == SEC_ADS_CONTROL_ACCESS) {
        return object_type_name;
    } else if (access_mask == SEC_ADS_READ_PROP) {
        return QString(QCoreApplication::translate("ad_security.cpp", "Read %1")).arg(object_type_name);
    } else if (access_mask == SEC_ADS_WRITE_PROP) {
        return QString(QCoreApplication::translate("ad_security.cpp", "Write %1")).arg(object_type_name);
    } else {
        const QHash<uint32_t, QString> common_right_name_map = {
            {SEC_ADS_GENERIC_ALL, QCoreApplication::translate("ad_security.cpp", "Full control")},
            {SEC_ADS_GENERIC_READ, QCoreApplication::translate("ad_security.cpp", "Read")},
            {SEC_ADS_GENERIC_WRITE, QCoreApplication::translate("ad_security.cpp", "Write")},
            {SEC_STD_DELETE, QCoreApplication::translate("ad_security.cpp", "Delete")},
            {SEC_ADS_CREATE_CHILD, QCoreApplication::translate("ad_security.cpp", "Create all child objects")},
            {SEC_ADS_DELETE_CHILD, QCoreApplication::translate("ad_security.cpp", "Delete all child objects")},
        };

        return common_right_name_map.value(access_mask, QCoreApplication::translate("ad_security.cpp", "<unknown right>"));
    }
}

void security_descriptor_add_right(security_descriptor *sd, AdConfig *adconfig, const QList<QString> &class_list, const QByteArray &trustee, const uint32_t access_mask, const QByteArray &object_type, const bool allow) {
    const QList<SecurityRight> superior_list = ad_security_get_superior_right_list(access_mask, object_type);
    for (const SecurityRight &superior : superior_list) {
        const bool opposite_superior_is_set = [&]() {
            const SecurityRightState state = security_descriptor_get_right(sd, trustee, superior.access_mask, superior.object_type);
            const SecurityRightStateType type = [&]() {
                // NOTE: opposite!
                if (!allow) {
                    return SecurityRightStateType_Allow;
                } else {
                    return SecurityRightStateType_Deny;
                }
            }();
            const bool out = state.get(SecurityRightStateInherited_No, type);

            return out;
        }();

        // NOTE: skip superior if it's not set, so that
        // we don't add opposite subordinate rights
        // when not needed
        if (!opposite_superior_is_set) {
            continue;
        }

        // Remove opposite superior
        security_descriptor_remove_right_base(sd, trustee, superior.access_mask, superior.object_type, !allow);

        // Add opposite superior subordinates
        const QList<SecurityRight> superior_subordinate_list = ad_security_get_subordinate_right_list(adconfig, superior.access_mask, superior.object_type, class_list);
        for (const SecurityRight &subordinate : superior_subordinate_list) {

            security_descriptor_add_right_base(sd, trustee, subordinate.access_mask, subordinate.object_type, !allow);
        }
    }

    // Remove subordinates
    const QList<SecurityRight> subordinate_list = ad_security_get_subordinate_right_list(adconfig, access_mask, object_type, class_list);
    for (const SecurityRight &subordinate : subordinate_list) {
        security_descriptor_remove_right_base(sd, trustee, subordinate.access_mask, subordinate.object_type, allow);
    }

    // Remove opposite
    security_descriptor_remove_right_base(sd, trustee, access_mask, object_type, !allow);

    // Remove opposite subordinates
    for (const SecurityRight &subordinate : subordinate_list) {
        security_descriptor_remove_right_base(sd, trustee, subordinate.access_mask, subordinate.object_type, !allow);
    }

    // Add target
    security_descriptor_add_right_base(sd, trustee, access_mask, object_type, allow);

    security_descriptor_sort_dacl(sd);
}

void security_descriptor_remove_right(security_descriptor *sd, AdConfig *adconfig, const QList<QString> &class_list, const QByteArray &trustee, const uint32_t access_mask, const QByteArray &object_type, const bool allow) {
    const QList<SecurityRight> target_superior_list = ad_security_get_superior_right_list(access_mask, object_type);

    // Remove superiors
    for (const SecurityRight &superior : target_superior_list) {
        const bool superior_is_set = [&]() {
            const SecurityRightState state = security_descriptor_get_right(sd, trustee, superior.access_mask, superior.object_type);
            const SecurityRightStateType type = [&]() {
                if (allow) {
                    return SecurityRightStateType_Allow;
                } else {
                    return SecurityRightStateType_Deny;
                }
            }();
            const bool out = state.get(SecurityRightStateInherited_No, type);

            return out;
        }();

        // NOTE: skip superior if it's not set, so that we don't add opposite subordinate rights when not needed
        if (!superior_is_set) {
            continue;
        }

        security_descriptor_remove_right_base(sd, trustee, superior.access_mask, superior.object_type, allow);

        const QList<SecurityRight> superior_subordinate_list = ad_security_get_subordinate_right_list(adconfig, superior.access_mask, superior.object_type, class_list);

        // Add opposite subordinate rights
        for (const SecurityRight &subordinate : superior_subordinate_list) {
            security_descriptor_add_right_base(sd, trustee, subordinate.access_mask, subordinate.object_type, allow);
        }
    }

    // Remove target right
    security_descriptor_remove_right_base(sd, trustee, access_mask, object_type, allow);

    // Add target subordinate rights
    const QList<SecurityRight> tarad_security_get_subordinate_right_list = ad_security_get_subordinate_right_list(adconfig, access_mask, object_type, class_list);
    for (const SecurityRight &subordinate : tarad_security_get_subordinate_right_list) {
        security_descriptor_add_right_base(sd, trustee, subordinate.access_mask, subordinate.object_type, allow);
    }

    security_descriptor_sort_dacl(sd);
}

QList<SecurityRight> ad_security_get_right_list_for_class(AdConfig *adconfig, const QList<QString> &class_list) {
    QList<SecurityRight> out = ad_security_get_common_rights() + ad_security_get_extended_rights_for_class(adconfig, class_list);
    return out;
}

QList<SecurityRight> ad_security_get_superior_right_list(const uint32_t access_mask, const QByteArray &object_type) {
    QList<SecurityRight> out;

    const bool object_present = !object_type.isEmpty();

    const SecurityRight generic_all = {SEC_ADS_GENERIC_ALL, QByteArray()};
    const SecurityRight generic_read = {SEC_ADS_GENERIC_READ, QByteArray()};
    const SecurityRight generic_write = {SEC_ADS_GENERIC_WRITE, QByteArray()};
    const SecurityRight all_extended_rights = {SEC_ADS_CONTROL_ACCESS, QByteArray()};

    // NOTE: order is important, because we want to
    // process "more superior" rights first. "Generic
    // all" is more superior than others.
    if (object_present) {
        if (access_mask == SEC_ADS_READ_PROP) {
            out.append(generic_all);
            out.append(generic_read);
        } else if (access_mask == SEC_ADS_WRITE_PROP) {
            out.append(generic_all);
            out.append(generic_write);
        } else if (access_mask == SEC_ADS_CONTROL_ACCESS) {
            out.append(generic_all);
            out.append(all_extended_rights);
        }
    } else {
        if (access_mask == SEC_ADS_GENERIC_READ || access_mask == SEC_ADS_GENERIC_WRITE) {
            out.append(generic_all);
        }
    }

    return out;
}

QList<SecurityRight> ad_security_get_subordinate_right_list(AdConfig *adconfig, const uint32_t access_mask, const QByteArray &object_type, const QList<QString> &class_list) {
    QList<SecurityRight> out;

    const bool object_present = !object_type.isEmpty();

    const QList<SecurityRight> right_list_for_target = ad_security_get_right_list_for_class(adconfig, class_list);

    for (const SecurityRight &right : right_list_for_target) {
        const bool match = [&]() {
            const bool right_object_present = !right.object_type.isEmpty();

            if (object_present) {
                return false;
            } else {
                if (access_mask == SEC_ADS_GENERIC_ALL) {
                    // All except full control
                    return (right.access_mask != access_mask);
                } else if (access_mask == SEC_ADS_GENERIC_READ) {
                    // All read property rights
                    return (right.access_mask == SEC_ADS_READ_PROP && right_object_present);
                } else if (access_mask == SEC_ADS_GENERIC_WRITE) {
                    // All write property rights
                    return (right.access_mask == SEC_ADS_WRITE_PROP && right_object_present);
                } else if (access_mask == SEC_ADS_CONTROL_ACCESS) {
                    // All extended rights
                    return (right.access_mask == SEC_ADS_CONTROL_ACCESS && right_object_present);
                } else {
                    return false;
                }
            }
        }();

        if (match) {
            out.append(right);
        }
    }

    return out;
}

void ad_security_replace_dacl(security_descriptor *sd, const QList<security_ace> &new_dacl) {

    // Free old dacl
    talloc_free(sd->dacl);
    sd->dacl = NULL;

    // Fill new dacl
    // NOTE: dacl_add() allocates new dacl
    for (const security_ace &ace : new_dacl) {
        security_descriptor_dacl_add(sd, &ace);
    }
}

// This f-n is only necessary to band-aid one problem
// with generic read.
uint32_t ad_security_map_access_mask(const uint32_t access_mask) {
    const bool is_generic_read = (access_mask == SEC_ADS_GENERIC_READ);

    if (is_generic_read) {
        return GENERIC_READ_FIXED;
    } else {
        return access_mask;
    }
}

// This simplified version of ace_compare() for
// verifying ACL order. Only includes necessary
// comparisons specified by Microsoft here:
// https://docs.microsoft.com/en-us/windows/win32/secauthz/order-of-aces-in-a-dacl
//
// TODO: currently missing one comparison:
//
// "Inherited ACE's are placed in the order in which
// they are inherited"
//
// Not a big problem because inherited ACE's are added
// to ACL by the server. Clients cannot manually add
// such ACE's, so theoretically their order should
// always be correct. But do implement this at some
// point, just in case. Using order requirements listed
// here:
int ace_compare_simplified(const security_ace &ace1, const security_ace &ace2) {
    bool b1;
    bool b2;

    /* If the ACEs are equal, we have nothing more to do. */
    if (security_ace_equal(&ace1, &ace2)) {
        return 0;
    }

    /* Inherited follow non-inherited */
    b1 = ((ace1.flags & SEC_ACE_FLAG_INHERITED_ACE) != 0);
    b2 = ((ace2.flags & SEC_ACE_FLAG_INHERITED_ACE) != 0);
    if (b1 != b2) {
        return (b1 ? 1 : -1);
    }

    /* Allowed ACEs follow denied ACEs */
    b1 = (ace1.type == SEC_ACE_TYPE_ACCESS_ALLOWED ||
          ace1.type == SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT);
    b2 = (ace2.type == SEC_ACE_TYPE_ACCESS_ALLOWED ||
          ace2.type == SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT);
    if (b1 != b2) {
        return (b1 ? 1 : -1);
    }

    return 0;
}

QList<SecurityRight> ad_security_get_common_rights() {
    QList<SecurityRight> out;

    for (const uint32_t &access_mask : common_rights_list) {
        SecurityRight right;
        right.access_mask = access_mask;
        right.object_type = QByteArray();

        out.append(right);
    }

    return out;
}

QList<SecurityRight> ad_security_get_extended_rights_for_class(AdConfig *adconfig, const QList<QString> &class_list) {
    QList<SecurityRight> out;

    const QList<QString> extended_rights_list = adconfig->get_extended_rights_list(class_list);
    for (const QString &rights : extended_rights_list) {
        const int valid_accesses = adconfig->get_rights_valid_accesses(rights);
        const QByteArray rights_guid = adconfig->get_right_guid(rights);
        const QList<uint32_t> access_mask_list = {
            SEC_ADS_CONTROL_ACCESS,
            SEC_ADS_READ_PROP,
            SEC_ADS_WRITE_PROP,
        };

        for (const uint32_t &access_mask : access_mask_list) {
            const bool mask_match = bitmask_is_set(valid_accesses, access_mask);

            if (mask_match) {
                SecurityRight right;
                right.access_mask = access_mask;
                right.object_type = rights_guid;

                out.append(right);
            }
        }
    }

    return out;
}
