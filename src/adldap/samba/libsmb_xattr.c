/* 
   Unix SMB/Netbios implementation.
   SMB client library implementation
   Copyright (C) Andrew Tridgell 1998
   Copyright (C) Richard Sharpe 2000, 2002
   Copyright (C) John Terpstra 2000
   Copyright (C) Tom Jansen (Ninja ISD) 2002 
   Copyright (C) Derrell Lipman 2003-2008
   Copyright (C) Jeremy Allison 2007, 2008

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * This file is a copy of private samba sources. Parts of it
 * were removed or edited.
 */

#include "samba/dom_sid.h"

#include <ndr.h>
#include <gen_ndr/security.h>
#include <string.h>

/*
 * Sort ACEs according to the documentation at
 * http://support.microsoft.com/kb/269175, at least as far as it defines the
 * order.
 */

int ace_compare(void const *ace1_ptr,
            void const *ace2_ptr)
{
    const struct security_ace *ace1 = ace1_ptr;
    const struct security_ace *ace2 = ace2_ptr;

    bool b1;
    bool b2;

    /* If the ACEs are equal, we have nothing more to do. */
    if (security_ace_equal(ace1, ace2)) {
    return 0;
    }

    /* Inherited follow non-inherited */
    b1 = ((ace1->flags & SEC_ACE_FLAG_INHERITED_ACE) != 0);
    b2 = ((ace2->flags & SEC_ACE_FLAG_INHERITED_ACE) != 0);
    if (b1 != b2) {
            return (b1 ? 1 : -1);
    }

    /*
     * What shall we do with AUDITs and ALARMs?  It's undefined.  We'll
     * sort them after DENY and ALLOW.
     */
    b1 = (ace1->type != SEC_ACE_TYPE_ACCESS_ALLOWED &&
          ace1->type != SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT &&
          ace1->type != SEC_ACE_TYPE_ACCESS_DENIED &&
          ace1->type != SEC_ACE_TYPE_ACCESS_DENIED_OBJECT);
    b2 = (ace2->type != SEC_ACE_TYPE_ACCESS_ALLOWED &&
          ace2->type != SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT &&
          ace2->type != SEC_ACE_TYPE_ACCESS_DENIED &&
          ace2->type != SEC_ACE_TYPE_ACCESS_DENIED_OBJECT);
    if (b1 != b2) {
            return (b1 ? 1 : -1);
    }

    /* Allowed ACEs follow denied ACEs */
    b1 = (ace1->type == SEC_ACE_TYPE_ACCESS_ALLOWED ||
          ace1->type == SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT);
    b2 = (ace2->type == SEC_ACE_TYPE_ACCESS_ALLOWED ||
          ace2->type == SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT);
    if (b1 != b2) {
            return (b1 ? 1 : -1);
    }

    /*
     * ACEs applying to an entity's object follow those applying to the
     * entity itself
     */
    b1 = (ace1->type == SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT ||
          ace1->type == SEC_ACE_TYPE_ACCESS_DENIED_OBJECT);
    b2 = (ace2->type == SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT ||
          ace2->type == SEC_ACE_TYPE_ACCESS_DENIED_OBJECT);
    if (b1 != b2) {
            return (b1 ? 1 : -1);
    }

    /*
     * If we get this far, the ACEs are similar as far as the
     * characteristics we typically care about (those defined by the
     * referenced MS document).  We'll now sort by characteristics that
     * just seems reasonable.
     */

    if (ace1->type != ace2->type) {
        return ace2->type - ace1->type;
        }

    if (dom_sid_compare(&ace1->trustee, &ace2->trustee)) {
        return dom_sid_compare(&ace1->trustee, &ace2->trustee);
        }

    if (ace1->flags != ace2->flags) {
        return ace1->flags - ace2->flags;
        }

    if (ace1->access_mask != ace2->access_mask) {
        return ace1->access_mask - ace2->access_mask;
        }

    if (ace1->size != ace2->size) {
        return ace1->size - ace2->size;
        }

    return memcmp(ace1, ace2, sizeof(struct security_ace));
}
