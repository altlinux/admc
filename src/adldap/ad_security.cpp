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

#include "ad_security.h"

#include "adldap.h"

#include "samba/ndr_security.h"
#include "samba/dom_sid.h"
#include "samba/libsmb_xattr.h"
#include "samba/security_descriptor.h"

#include "ad_filter.h"

#include <QDebug>

// TODO: values of SEC_ADS_GENERIC_READ and
// SEC_ADS_GENERIC_WRITE constants don't match with the bits
// that ADUC sets when you enable those permissions in
// security tab. There are some extra bits in these
// constants, took them out as a quick fix.
const QHash<AcePermission, uint32_t> ace_permission_to_mask_map = {
    {AcePermission_FullControl, SEC_ADS_GENERIC_ALL},
    // {AcePermission_Read, SEC_ADS_GENERIC_READ},
    {AcePermission_Read, (SEC_STD_READ_CONTROL | SEC_ADS_LIST | SEC_ADS_READ_PROP)},
    // {AcePermission_Write, SEC_ADS_GENERIC_WRITE},
    {AcePermission_Write, (SEC_ADS_SELF_WRITE | SEC_ADS_WRITE_PROP)},
    {AcePermission_CreateChild, SEC_ADS_CREATE_CHILD},
    {AcePermission_DeleteChild, SEC_ADS_DELETE_CHILD},
    {AcePermission_AllowedToAuthenticate, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_ChangePassword, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_ReceiveAs, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_ResetPassword, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_SendAs, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_ReadAccountRestrictions, SEC_ADS_READ_PROP},
    {AcePermission_WriteAccountRestrictions, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadGeneralInfo, SEC_ADS_READ_PROP},
    {AcePermission_WriteGeneralInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadGroupMembership, SEC_ADS_READ_PROP},
    {AcePermission_ReadLogonInfo, SEC_ADS_READ_PROP},
    {AcePermission_WriteLogonInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadPersonalInfo, SEC_ADS_READ_PROP},
    {AcePermission_WritePersonalInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadPhoneAndMailOptions, SEC_ADS_READ_PROP},
    {AcePermission_WritePhoneAndMailOptions, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadPrivateInfo, SEC_ADS_READ_PROP},
    {AcePermission_WritePrivateInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadPublicInfo, SEC_ADS_READ_PROP},
    {AcePermission_WritePublicInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadRemoteAccessInfo, SEC_ADS_READ_PROP},
    {AcePermission_WriteRemoteAccessInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadTerminalServerLicenseServer, SEC_ADS_READ_PROP},
    {AcePermission_WriteTerminalServerLicenseServer, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadWebInfo, SEC_ADS_READ_PROP},
    {AcePermission_WriteWebInfo, SEC_ADS_WRITE_PROP},
};

// NOTE: store right's cn value here, then search for it to
// get right's guid, which is compared to ace type.
const QHash<AcePermission, QString> ace_permission_to_type_map = {
    {AcePermission_AllowedToAuthenticate, "Allowed-To-Authenticate"},
    {AcePermission_ChangePassword, "User-Change-Password"},
    {AcePermission_ReceiveAs, "Receive-As"},
    {AcePermission_ResetPassword, "User-Force-Change-Password"},
    {AcePermission_SendAs, "Send-As"},
    {AcePermission_ReadAccountRestrictions, "User-Account-Restrictions"},
    {AcePermission_WriteAccountRestrictions, "User-Account-Restrictions"},
    {AcePermission_ReadGeneralInfo, "General-Information"},
    {AcePermission_WriteGeneralInfo, "General-Information"},
    {AcePermission_ReadGroupMembership, "Membership"},
    {AcePermission_ReadLogonInfo, "User-Logon"},
    {AcePermission_WriteLogonInfo, "User-Logon"},
    {AcePermission_ReadPersonalInfo, "Personal-Information"},
    {AcePermission_WritePersonalInfo, "Personal-Information"},
    {AcePermission_ReadPhoneAndMailOptions, "Email-Information"},
    {AcePermission_WritePhoneAndMailOptions, "Email-Information"},
    {AcePermission_ReadPrivateInfo, "Private-Information"},
    {AcePermission_WritePrivateInfo, "Private-Information"},
    {AcePermission_ReadPublicInfo, "Public-Information"},
    {AcePermission_WritePublicInfo, "Public-Information"},
    {AcePermission_ReadRemoteAccessInfo, "RAS-Information"},
    {AcePermission_WriteRemoteAccessInfo, "RAS-Information"},
    {AcePermission_ReadTerminalServerLicenseServer, "Terminal-Server-License-Server"},
    {AcePermission_WriteTerminalServerLicenseServer, "Terminal-Server-License-Server"},
    {AcePermission_ReadWebInfo, "Web-Information"},
    {AcePermission_WriteWebInfo, "Web-Information"}
};

const QList<AcePermission> all_permissions_list =
[]() {
    QList<AcePermission> out;

    for (int permission_i = 0; permission_i < AcePermission_COUNT; permission_i++) {
        const AcePermission permission = (AcePermission) permission_i;
        out.append(permission);
    }

    return out;
}();

const QSet<AcePermission> all_permissions = all_permissions_list.toSet();

QSet<AcePermission> get_permission_set(const uint32_t mask) {
    QSet<AcePermission> out;

    for (const AcePermission &permission : all_permissions) {
        const uint32_t this_mask = ace_permission_to_mask_map[permission];

        if (this_mask == mask) {
            out.insert(permission);
        }
    }

    return out;
}

const QSet<AcePermission> access_permissions = get_permission_set(SEC_ADS_CONTROL_ACCESS);
const QSet<AcePermission> read_prop_permissions = get_permission_set(SEC_ADS_READ_PROP);
const QSet<AcePermission> write_prop_permissions = get_permission_set(SEC_ADS_WRITE_PROP);

SecurityDescriptor::SecurityDescriptor(const QByteArray &descriptor_bytes) {
    DATA_BLOB blob = data_blob_const(descriptor_bytes.data(), descriptor_bytes.size());

    data = talloc(NULL, struct security_descriptor);

    ndr_pull_struct_blob(&blob, data, data, (ndr_pull_flags_fn_t)ndr_pull_security_descriptor);
}

SecurityDescriptor::~SecurityDescriptor() {
    talloc_free(data);
}

QList<QByteArray> SecurityDescriptor::get_trustee_list() const {
    QSet<QByteArray> out;

    for (security_ace *ace : dacl()) {
        const QByteArray trustee_bytes = dom_sid_to_bytes(ace->trustee);

        out.insert(trustee_bytes);
    }

    return out.toList();
}

QList<security_ace *> SecurityDescriptor::dacl() const {
    QList<security_ace *> out;

    for (uint32_t i = 0; i < data->dacl->num_aces; i++) {
        security_ace *ace = &data->dacl->aces[i];
        out.append(ace);
    }

    return out;
}

void SecurityDescriptor::print_acl(const QByteArray &trustee) const {
    TALLOC_CTX *tmp_ctx = talloc_new(NULL);

    const QList<security_ace *> ace_list =
    [&]() {
        QList<security_ace *> out;

        for (security_ace *ace : dacl()) {
            const QByteArray this_trustee_bytes = dom_sid_to_bytes(ace->trustee);

            const bool trustee_match = (this_trustee_bytes == trustee);
            if (trustee_match) {
                out.append(ace);
            }
        }

        return out;
    }();

    for (security_ace *ace : ace_list) {
        const QString ace_string =
        [&]() {
            char *ace_cstr = ndr_print_struct_string(tmp_ctx, (ndr_print_fn_t)ndr_print_security_ace,
                "ace", ace);

            const QString out = QString(ace_cstr);

            return out;
        }();

        qDebug().noquote() << ace_string;
    }

    talloc_free(tmp_ctx);
}

security_descriptor *SecurityDescriptor::get_data() const {
    return data;
}

QByteArray dom_sid_to_bytes(const dom_sid &sid) {
    const QByteArray bytes = QByteArray((char *) &sid, sizeof(struct dom_sid));

    return bytes;
}

QString ad_security_get_trustee_name(AdInterface &ad, const QByteArray &trustee) {
    // TODO: not sure if these are supposed to be translated?
    static const QHash<QString, QString> trustee_name_map = {
        {SID_WORLD_DOMAIN, "Everyone in Domain"},
        {SID_WORLD, "Everyone"},
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

    const QString trustee_string = object_sid_display_value(trustee);

    if (trustee_name_map.contains(trustee_string)) {
        return trustee_name_map[trustee_string];
    } else {
        // Try to get name of trustee by finding it's DN
        const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_SID, trustee_string);
        const QList<QString> attributes = {
            ATTRIBUTE_DISPLAY_NAME,
            ATTRIBUTE_SAMACCOUNT_NAME,
        };
        const auto trustee_search = ad.search(ad.adconfig()->domain_head(), SearchScope_All, filter, QList<QString>());
        if (!trustee_search.isEmpty()) {
            // NOTE: this is some weird name selection logic
            // but that's how microsoft does it. Maybe need
            // to use this somewhere else as well?
            const QString name =
            [&]() {
                const AdObject object = trustee_search.values()[0];

                if (object.contains(ATTRIBUTE_DISPLAY_NAME)) {
                    return object.get_string(ATTRIBUTE_DISPLAY_NAME);
                } else if (object.contains(ATTRIBUTE_SAMACCOUNT_NAME)) {
                    return object.get_string(ATTRIBUTE_SAMACCOUNT_NAME);
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

bool attribute_replace_security_descriptor(AdInterface &ad, const QString &dn, const QHash<QByteArray, QHash<AcePermission, PermissionState>> &descriptor_state_arg) {
    const QByteArray new_descriptor_bytes =
    [&]() {
        // Remove redundancy from permission state
        const QHash<QByteArray, QHash<AcePermission, PermissionState>> state =
        [&]() {
            QHash<QByteArray, QHash<AcePermission, PermissionState>> out;

            out = descriptor_state_arg;

            // Remove child permission states. For example if
            // "Read" is allowed, then there's no need to
            // include any other state for "read prop"
            // permissions.
            for (const QByteArray &trustee : out.keys()) {
                const bool full_control = out[trustee].contains(AcePermission_FullControl) && (out[trustee][AcePermission_FullControl] != PermissionState_None);
                const bool read = out[trustee].contains(AcePermission_Read) && (out[trustee][AcePermission_Read] != PermissionState_None);
                const bool write = out[trustee].contains(AcePermission_Write) && (out[trustee][AcePermission_Write] != PermissionState_None);

                if (full_control) {
                    for (const AcePermission &permission : all_permissions) {
                        if (permission != AcePermission_FullControl) {
                            out[trustee].remove(permission);
                        }
                    }
                } else if (read) {
                    for (const AcePermission &permission : read_prop_permissions) {
                        if (permission != AcePermission_Read) {
                            out[trustee].remove(permission);
                        }
                    }
                } else if (write) {
                    for (const AcePermission &permission : write_prop_permissions) {
                        if (permission != AcePermission_Read) {
                            out[trustee].remove(permission);
                        }
                    }
                }
            }

            return out;
        }();

        TALLOC_CTX* tmp_ctx = talloc_new(NULL);

        // Use original sd as base, only remaking the dacl
        const AdObject object = ad.search_object(dn, {ATTRIBUTE_SECURITY_DESCRIPTOR});
        const QByteArray old_descriptor_bytes = object.get_value(ATTRIBUTE_SECURITY_DESCRIPTOR);
        const SecurityDescriptor old_sd = SecurityDescriptor(old_descriptor_bytes);
        security_descriptor *sd = old_sd.get_data();

        // Generate new dacl
        const QList<security_ace *> dacl_qlist =
        [&]() {
            QList<security_ace *> out;

            for (const QByteArray &trustee : state.keys()) {
                const QHash<AcePermission, PermissionState> permission_map = state[trustee];

                for (const AcePermission &permission : permission_map.keys()) {
                    const PermissionState permission_state = permission_map[permission];

                    if (permission_state == PermissionState_None) {
                        continue;
                    }

                    struct security_ace *ace = talloc(tmp_ctx, struct security_ace);

                    const bool object_present = ace_permission_to_type_map.contains(permission);

                    ace->type =
                    [&]() {
                        if (permission_state == PermissionState_Allowed) {
                            if (object_present) {
                                return SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT;
                            } else {
                                return SEC_ACE_TYPE_ACCESS_ALLOWED;
                            }
                        } else if (permission_state == PermissionState_Denied) {
                            if (object_present) {
                                return SEC_ACE_TYPE_ACCESS_DENIED_OBJECT;
                            } else {
                                return SEC_ACE_TYPE_ACCESS_DENIED;
                            }
                        }

                        return SEC_ACE_TYPE_ACCESS_ALLOWED;
                    }();

                    // TODO: these flags should be set to something
                    // in some cases, for now just 0
                    ace->flags = 0x00;
                    ace->access_mask = ace_permission_to_mask_map[permission];
                    ace->object.object.flags =
                    [&]() {
                        if (object_present) {
                            return SEC_ACE_OBJECT_TYPE_PRESENT;
                        } else {
                            return 0;
                        }
                    }();
                    ace->object.object.type.type =
                    [&]() {
                        if (object_present) {
                            const QString type_name_string = ace_permission_to_type_map[permission];
                            const QString type_string = ad.adconfig()->get_right_guid(type_name_string);
                            const QByteArray type_bytes = guid_string_to_bytes(type_string);

                            struct GUID guid;
                            memcpy(&guid, type_bytes.data(), sizeof(GUID));

                            return guid;
                        } else {
                            return GUID();
                        }
                    }();
                    memcpy(&ace->trustee, trustee.data(), sizeof(dom_sid));

                    out.append(ace);
                }
            }

            return out;
        }();

        // Replace dacl
        talloc_free(sd->dacl);
        sd->dacl = NULL;
        for (security_ace *ace : dacl_qlist) {
            security_descriptor_dacl_add(sd, ace);
        }
        qsort(sd->dacl->aces, sd->dacl->num_aces, sizeof(security_ace), ace_compare);

        DATA_BLOB blob;
        ndr_push_struct_blob(&blob, tmp_ctx, sd, (ndr_push_flags_fn_t)ndr_push_security_descriptor);

        const QByteArray out = QByteArray((char *) blob.data, blob.length);

        talloc_free(tmp_ctx);

        return out;
    }();

    const bool apply_success = ad.attribute_replace_value(dn, ATTRIBUTE_SECURITY_DESCRIPTOR, new_descriptor_bytes);

    return apply_success;
}
