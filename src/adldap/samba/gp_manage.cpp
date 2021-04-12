/*
 *  Unix SMB/CIFS implementation.
 *  Group Policy Object Support
 *  Copyright (C) Wilco Baan Hofman 2010
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This file is a copy of private files from samba source.
 * Parts of it were removed or edited.
 */

#include "gp_manage.h"

#include "dom_sid.h"

#include <string.h>

#include <QDebug>

/*
 * This file is a copy of private samba sources. Parts of it
 * were removed or edited.
 */

bool security_descriptor_acl_add(struct security_descriptor *sd,
                        bool add_to_sacl,
                        const struct security_ace *ace)
{
    struct security_acl *acl = NULL;

    if (add_to_sacl) {
        acl = sd->sacl;
    } else {
        acl = sd->dacl;
    }

    if (acl == NULL) {
        acl = talloc(sd, struct security_acl);
        if (acl == NULL) {
            // return NT_STATUS_NO_MEMORY;
            return false;
        }
        acl->revision = SECURITY_ACL_REVISION_NT4;
        acl->size     = 0;
        acl->num_aces = 0;
        acl->aces     = NULL;
    }

    acl->aces = talloc_realloc(acl, acl->aces,
                   struct security_ace, acl->num_aces+1);
    if (acl->aces == NULL) {
        // return NT_STATUS_NO_MEMORY;
        return false;
    }

    acl->aces[acl->num_aces] = *ace;

    switch (acl->aces[acl->num_aces].type) {
    case SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT:
    case SEC_ACE_TYPE_ACCESS_DENIED_OBJECT:
    case SEC_ACE_TYPE_SYSTEM_AUDIT_OBJECT:
    case SEC_ACE_TYPE_SYSTEM_ALARM_OBJECT:
        acl->revision = SECURITY_ACL_REVISION_ADS;
        break;
    default:
        break;
    }

    acl->num_aces++;

    if (add_to_sacl) {
        sd->sacl = acl;
        sd->type |= SEC_DESC_SACL_PRESENT;
    } else {
        sd->dacl = acl;
        sd->type |= SEC_DESC_DACL_PRESENT;
    }

    // return NT_STATUS_OK;
    return true;
}

/*
  add an ACE to the SACL of a security_descriptor
*/

bool security_descriptor_sacl_add(struct security_descriptor *sd,
                      const struct security_ace *ace)
{
    return security_descriptor_acl_add(sd, true, ace);
}

/*
  add an ACE to the DACL of a security_descriptor
*/

bool security_descriptor_dacl_add(struct security_descriptor *sd,
                      const struct security_ace *ace)
{
    return security_descriptor_acl_add(sd, false, ace);
}

uint32_t gp_ads_to_dir_access_mask(uint32_t access_mask)
{
    uint32_t fs_mask;

    /* Copy the standard access mask */
    fs_mask = access_mask & 0x001F0000;

    /* When READ_PROP and LIST_CONTENTS are set, read access is granted on the GPT */
    if (access_mask & SEC_ADS_READ_PROP && access_mask & SEC_ADS_LIST) {
        fs_mask |= SEC_STD_SYNCHRONIZE | SEC_DIR_LIST | SEC_DIR_READ_ATTRIBUTE |
                SEC_DIR_READ_EA | SEC_DIR_TRAVERSE;
    }

    /* When WRITE_PROP is set, full write access is granted on the GPT */
    if (access_mask & SEC_ADS_WRITE_PROP) {
        fs_mask |= SEC_STD_SYNCHRONIZE | SEC_DIR_WRITE_ATTRIBUTE |
                SEC_DIR_WRITE_EA | SEC_DIR_ADD_FILE |
                SEC_DIR_ADD_SUBDIR;
    }

    /* Map CREATE_CHILD to add file and add subdir */
    if (access_mask & SEC_ADS_CREATE_CHILD)
        fs_mask |= SEC_DIR_ADD_FILE | SEC_DIR_ADD_SUBDIR;

    /* Map ADS delete child to dir delete child */
    if (access_mask & SEC_ADS_DELETE_CHILD)
        fs_mask |= SEC_DIR_DELETE_CHILD;

    return fs_mask;
}

struct security_acl *security_acl_dup(TALLOC_CTX *mem_ctx,
                         const struct security_acl *oacl)
{
    struct security_acl *nacl;

    if (oacl == NULL) {
        return NULL;
    }

    if (oacl->aces == NULL && oacl->num_aces > 0) {
        return NULL;
    }

    nacl = talloc (mem_ctx, struct security_acl);
    if (nacl == NULL) {
        return NULL;
    }

    *nacl = (struct security_acl) {
        .revision = oacl->revision,
        .size     = oacl->size,
        .num_aces = oacl->num_aces,
    };
    if (nacl->num_aces == 0) {
        return nacl;
    }

    nacl->aces = (struct security_ace *)talloc_memdup (nacl, oacl->aces, sizeof(struct security_ace) * oacl->num_aces);
    if (nacl->aces == NULL) {
        goto failed;
    }

    return nacl;

 failed:
    talloc_free (nacl);
    return NULL;
    
}

bool gp_create_gpt_security_descriptor(TALLOC_CTX *mem_ctx, struct security_descriptor *ds_sd, struct security_descriptor **ret) {
    struct security_descriptor *fs_sd;

    /* Allocate the file system security descriptor */
    fs_sd = talloc(mem_ctx, struct security_descriptor);

    /* Copy the basic information from the directory server security descriptor */
    fs_sd->owner_sid = (dom_sid *) talloc_memdup(fs_sd, ds_sd->owner_sid, sizeof(struct dom_sid));
    if (fs_sd->owner_sid == NULL) {
        TALLOC_FREE(fs_sd);
        qDebug() << "Failed to allocated owner sid";
        return false;
    }

    fs_sd->group_sid = (dom_sid *) talloc_memdup(fs_sd, ds_sd->group_sid, sizeof(struct dom_sid));
    if (fs_sd->group_sid == NULL) {
        TALLOC_FREE(fs_sd);
        qDebug() << "Failed to allocated group sid";
        return false;
    }

    fs_sd->type = ds_sd->type;
    fs_sd->revision = ds_sd->revision;

    /* Copy the sacl */
    fs_sd->sacl = security_acl_dup(fs_sd, ds_sd->sacl);
    if (fs_sd->sacl == NULL) {
        TALLOC_FREE(fs_sd);
        qDebug() << "Failed to allocated sacl";
        return false;
    }

    /* Copy the dacl */
    fs_sd->dacl = talloc_zero(fs_sd, struct security_acl);
    if (fs_sd->dacl == NULL) {
        TALLOC_FREE(fs_sd);
        qDebug() << "Failed to allocated dacl";
        return false;
    }

    for (uint32_t i = 0; i < ds_sd->dacl->num_aces; i++) {
        const char *trustee = dom_sid_string(fs_sd, &ds_sd->dacl->aces[i].trustee);
        struct security_ace *ace;

        /* Don't add the allow for SID_BUILTIN_PREW2K */
        if (!(ds_sd->dacl->aces[i].type & SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT) && strcmp(trustee, SID_BUILTIN_PREW2K) == 0) {
            talloc_free((void *) trustee);
            continue;
        }

        /* Copy the ace from the directory server security descriptor */
        ace = (security_ace *) talloc_memdup(fs_sd, &ds_sd->dacl->aces[i], sizeof(struct security_ace));
        if (ace == NULL) {
            TALLOC_FREE(fs_sd);
            qDebug() << "Failed to allocated ace";
            return false;
        }

        /* Set specific inheritance flags for within the GPO */
        ace->flags |= SEC_ACE_FLAG_OBJECT_INHERIT | SEC_ACE_FLAG_CONTAINER_INHERIT;
        if (strcmp(trustee, SID_CREATOR_OWNER) == 0) {
            ace->flags |= SEC_ACE_FLAG_INHERIT_ONLY;
        }

        /* Get a directory access mask from the assigned access mask on the LDAP object */
        ace->access_mask = gp_ads_to_dir_access_mask(ace->access_mask);

        /* Add the ace to the security descriptor DACL */
        const bool dacl_add_success = security_descriptor_dacl_add(fs_sd, ace);
        if (!dacl_add_success) {
            qDebug() << "Failed to add a dacl to file system security descriptor";
            return false;
        }

        /* Clean up the allocated data in this iteration */
        talloc_free((void *) trustee);
    }

    *ret = fs_sd;
    return true;
}
