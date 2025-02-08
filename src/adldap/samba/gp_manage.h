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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GP_MANAGE_H
#define GP_MANAGE_H

// time.h and unistd.h Are needed before including ndr.h when building against old samba (4.16)
#include <time.h>
#include <unistd.h>

#include <ndr.h>
#include <gen_ndr/security.h>
#include <talloc.h>

NTSTATUS gp_create_gpt_security_descriptor(TALLOC_CTX *mem_ctx, struct security_descriptor *ds_sd, struct security_descriptor **ret);

#endif /* GP_MANAGE_H */

#ifdef __cplusplus
}
#endif
