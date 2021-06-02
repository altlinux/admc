/*
   Unix SMB/CIFS implementation.
   Samba utility functions

   Copyright (C) Stefan (metze) Metzmacher  2002-2004
   Copyright (C) Andrew Tridgell        1992-2004
   Copyright (C) Jeremy Allison         1999

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

#include "dom_sid.h"

#include "replace.h"

/*****************************************************************
 Compare the auth portion of two sids.
*****************************************************************/

int dom_sid_compare_auth(const struct dom_sid *sid1,
             const struct dom_sid *sid2)
{
    int i;

    if (sid1 == sid2)
        return 0;
    if (!sid1)
        return -1;
    if (!sid2)
        return 1;

    if (sid1->sid_rev_num != sid2->sid_rev_num)
        return sid1->sid_rev_num - sid2->sid_rev_num;

    for (i = 0; i < 6; i++)
        if (sid1->id_auth[i] != sid2->id_auth[i])
            return sid1->id_auth[i] - sid2->id_auth[i];

    return 0;
}

/*****************************************************************
 Compare two sids.
*****************************************************************/

int dom_sid_compare(const struct dom_sid *sid1, const struct dom_sid *sid2)
{
    int i;

    if (sid1 == sid2)
        return 0;
    if (!sid1)
        return -1;
    if (!sid2)
        return 1;

    /* Compare most likely different rids, first: i.e start at end */
    if (sid1->num_auths != sid2->num_auths)
        return sid1->num_auths - sid2->num_auths;

    for (i = sid1->num_auths-1; i >= 0; --i)
        if (sid1->sub_auths[i] != sid2->sub_auths[i])
            return sid1->sub_auths[i] - sid2->sub_auths[i];

    return dom_sid_compare_auth(sid1, sid2);
}

/*****************************************************************
 Compare two sids.
*****************************************************************/

bool dom_sid_equal(const struct dom_sid *sid1, const struct dom_sid *sid2)
{
    return dom_sid_compare(sid1, sid2) == 0;
}

/*
  Convert a dom_sid to a string, printing into a buffer. Return the
  string length. If it overflows, return the string length that would
  result (buflen needs to be +1 for the terminating 0).
*/
static int dom_sid_string_buf(const struct dom_sid *sid, char *buf, int buflen)
{
    int i, ofs, ret;
    uint64_t ia;

    if (!sid) {
        return strlcpy(buf, "(NULL SID)", buflen);
    }

    ia = ((uint64_t)sid->id_auth[5]) +
        ((uint64_t)sid->id_auth[4] << 8 ) +
        ((uint64_t)sid->id_auth[3] << 16) +
        ((uint64_t)sid->id_auth[2] << 24) +
        ((uint64_t)sid->id_auth[1] << 32) +
        ((uint64_t)sid->id_auth[0] << 40);

    ret = snprintf(buf, buflen, "S-%"PRIu8"-", sid->sid_rev_num);
    if (ret < 0) {
        return ret;
    }
    ofs = ret;

    if (ia >= UINT32_MAX) {
        ret = snprintf(buf+ofs, MAX(buflen-ofs, 0), "0x%"PRIx64, ia);
    } else {
        ret = snprintf(buf+ofs, MAX(buflen-ofs, 0), "%"PRIu64, ia);
    }
    if (ret < 0) {
        return ret;
    }
    ofs += ret;

    for (i = 0; i < sid->num_auths; i++) {
        ret = snprintf(
            buf+ofs,
            MAX(buflen-ofs, 0),
            "-%"PRIu32,
            sid->sub_auths[i]);
        if (ret < 0) {
            return ret;
        }
        ofs += ret;
    }
    return ofs;
}

/*
  convert a dom_sid to a string
*/
char *dom_sid_string(TALLOC_CTX *mem_ctx, const struct dom_sid *sid)
{
    char buf[DOM_SID_STR_BUFLEN];
    char *result;
    int len;

    len = dom_sid_string_buf(sid, buf, sizeof(buf));

    if ((len < 0) || (len+1 > sizeof(buf))) {
        return talloc_strdup(mem_ctx, "(SID ERR)");
    }

    /*
     * Avoid calling strlen (via talloc_strdup), we already have
     * the length
     */
    result = (char *)talloc_memdup(mem_ctx, buf, len+1);
    if (result == NULL) {
        return NULL;
    }

    /*
     * beautify the talloc_report output
     */
    talloc_set_name_const(result, result);
    return result;
}

char *dom_sid_str_buf(const struct dom_sid *sid, struct dom_sid_buf *dst)
{
    int ret;
    ret = dom_sid_string_buf(sid, dst->buf, sizeof(dst->buf));
    if ((ret < 0) || (ret >= sizeof(dst->buf))) {
        strlcpy(dst->buf, "(INVALID SID)", sizeof(dst->buf));
    }
    return dst->buf;
}
