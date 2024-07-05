/*
   Unix SMB/CIFS implementation.
   Samba utility functions

   Copyright (C) 2009 Jelmer Vernooij <jelmer@samba.org>

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

#ifndef _ACE_H_
#define _ACE_H_

#include <gen_ndr/security.h>

/*
 * This file is a copy of private samba sources. Parts of it
 * were removed or edited.
 */

bool sec_ace_object(uint8_t type);
bool sec_ace_callback(uint8_t type);
bool sec_ace_resource(uint8_t type);

#endif /*_ACE_H_*/

