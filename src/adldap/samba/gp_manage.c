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
#include "security_descriptor.h"

#include <string.h>

/*
 * This file is a copy of private samba sources. Parts of it
 * were removed or edited.
 */

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

NTSTATUS gp_create_gpt_security_descriptor(TALLOC_CTX *mem_ctx, struct security_descriptor *ds_sd, struct security_descriptor **ret) {
    struct security_descriptor *fs_sd;
    NTSTATUS status;
    uint32_t i;

    /* Allocate the file system security descriptor */
    fs_sd = talloc(mem_ctx, struct security_descriptor);
    NT_STATUS_HAVE_NO_MEMORY(fs_sd);

    /* Copy the basic information from the directory server security descriptor */
    fs_sd->owner_sid = talloc_memdup(fs_sd, ds_sd->owner_sid, sizeof(struct dom_sid));
    if (fs_sd->owner_sid == NULL) {
        TALLOC_FREE(fs_sd);
        return NT_STATUS_NO_MEMORY;
    }

    // NOTE: group sid of domain object by default is NULL,
    // so just copy the owner sid which is by default
    // "Domain Admins"
    fs_sd->group_sid = fs_sd->owner_sid;

    fs_sd->type = ds_sd->type;
    fs_sd->revision = ds_sd->revision;

    /* Copy the sacl */
    fs_sd->sacl = security_acl_dup(fs_sd, ds_sd->sacl);
    if (fs_sd->sacl == NULL) {
        TALLOC_FREE(fs_sd);
        return NT_STATUS_NO_MEMORY;
    }

    /* Copy the dacl */
    fs_sd->dacl = talloc_zero(fs_sd, struct security_acl);
    if (fs_sd->dacl == NULL) {
        TALLOC_FREE(fs_sd);
        return NT_STATUS_NO_MEMORY;
    }

    for (i = 0; i < ds_sd->dacl->num_aces; i++) {
        char *trustee = dom_sid_string(fs_sd, &ds_sd->dacl->aces[i].trustee);
        struct security_ace *ace;

        /* Don't add the allow for SID_BUILTIN_PREW2K */
        if (!(ds_sd->dacl->aces[i].type & SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT) &&
                strcmp(trustee, SID_BUILTIN_PREW2K) == 0) {
            talloc_free(trustee);
            continue;
        }

        /* Copy the ace from the directory server security descriptor */
        ace = talloc_memdup(fs_sd, &ds_sd->dacl->aces[i], sizeof(struct security_ace));
        if (ace == NULL) {
            TALLOC_FREE(fs_sd);
            return NT_STATUS_NO_MEMORY;
        }

        /* Set specific inheritance flags for within the GPO */
        ace->flags |= SEC_ACE_FLAG_OBJECT_INHERIT | SEC_ACE_FLAG_CONTAINER_INHERIT;
        if (strcmp(trustee, SID_CREATOR_OWNER) == 0) {
            ace->flags |= SEC_ACE_FLAG_INHERIT_ONLY;
        }

        /* Get a directory access mask from the assigned access mask on the LDAP object */
        ace->access_mask = gp_ads_to_dir_access_mask(ace->access_mask);

        // NOTE: ACE may become empty when it's access mask
        // is transformed for sysvol format. In that case,
        // skip it.
        const bool ace_is_empty = (ace->access_mask == 0x00000000);
        if (ace_is_empty) {
            talloc_free(trustee);
            continue;
        }

        /* Add the ace to the security descriptor DACL */
        status = security_descriptor_dacl_add(fs_sd, ace);
        if (!NT_STATUS_IS_OK(status)) {
            // DEBUG(0, ("Failed to add a dacl to file system security descriptor\n"));
            return status;
        }

        /* Clean up the allocated data in this iteration */
        talloc_free(trustee);
    }

    *ret = fs_sd;
    return NT_STATUS_OK;
}
