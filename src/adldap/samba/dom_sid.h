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
 * This file is a copy of private samba sources. Parts of it
 * were removed or edited.
 */

#ifndef DOM_SID_H
#define DOM_SID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <talloc.h>
#include <util/data_blob.h>
#include <gen_ndr/security.h>

#define DOM_SID_STR_BUFLEN (15*11+25)
char *dom_sid_string(TALLOC_CTX *mem_ctx, const struct dom_sid *sid);

struct dom_sid_buf { char buf[DOM_SID_STR_BUFLEN]; };
char *dom_sid_str_buf(const struct dom_sid *sid, struct dom_sid_buf *dst);

#ifdef __cplusplus
}
#endif

#endif /* DOM_SID_H */
