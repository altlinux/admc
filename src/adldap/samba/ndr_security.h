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

#ifndef NDR_SECURITY_H
#define NDR_SECURITY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ndr.h>
#include <gen_ndr/security.h>

enum ndr_err_code
ndr_security_pull_security_descriptor(struct ndr_pull *ndr,
                                    int ndr_flags,
                                    struct security_descriptor *r);

#ifdef __cplusplus
}
#endif

#endif /* NDR_SECURITY_H */
