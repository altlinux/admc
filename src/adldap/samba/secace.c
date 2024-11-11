/*
 *  Unix SMB/Netbios implementation.
 *  struct security_ace handling functions
 *  Copyright (C) Andrew Tridgell              1992-1998,
 *  Copyright (C) Jeremy R. Allison            1995-2003.
 *  Copyright (C) Luke Kenneth Casson Leighton 1996-1998,
 *  Copyright (C) Paul Ashton                  1997-1998.
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

#include "secace.h"

/**
 * Check if ACE has OBJECT type.
 */
bool sec_ace_object(uint8_t type)
{
	if (type == SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT ||
	    type == SEC_ACE_TYPE_ACCESS_DENIED_OBJECT ||
	    type == SEC_ACE_TYPE_SYSTEM_AUDIT_OBJECT ||
	    type == SEC_ACE_TYPE_SYSTEM_ALARM_OBJECT ||
	    type == SEC_ACE_TYPE_ACCESS_ALLOWED_CALLBACK_OBJECT ||
	    type == SEC_ACE_TYPE_ACCESS_DENIED_CALLBACK_OBJECT ||
	    type == SEC_ACE_TYPE_SYSTEM_AUDIT_CALLBACK_OBJECT) {
		/*
		 * MS-DTYP has a reserved value for
		 * SEC_ACE_TYPE_SYSTEM_ALARM_CALLBACK_OBJECT, but we
		 * don't assume that it will be an object ACE just
		 * because it sounds like one.
		 */
		return true;
	}
	return false;
}

/**
 * Check if ACE is a CALLBACK type, which means it will have a blob of data at
 * the end.
 */
bool sec_ace_callback(uint8_t type)
{
	if (type == SEC_ACE_TYPE_ACCESS_ALLOWED_CALLBACK ||
	    type == SEC_ACE_TYPE_ACCESS_DENIED_CALLBACK ||
	    type == SEC_ACE_TYPE_ACCESS_ALLOWED_CALLBACK_OBJECT ||
	    type == SEC_ACE_TYPE_ACCESS_DENIED_CALLBACK_OBJECT ||
	    type == SEC_ACE_TYPE_SYSTEM_AUDIT_CALLBACK ||
	    type == SEC_ACE_TYPE_SYSTEM_AUDIT_CALLBACK_OBJECT) {
	    /*
	     * While SEC_ACE_TYPE_SYSTEM_ALARM_CALLBACK and
	     * SEC_ACE_TYPE_SYSTEM_ALARM_CALLBACK_OBJECT sound like
	     * callback types, they are reserved values in MS-DTYP,
	     * and their eventual use is not defined.
	     */
		return true;
	}
	return false;
}

/**
 * Check if an ACE type is resource attribute, which means it will
 * have a blob of data at the end defining an attribute on the object.
 * Resource attribute ACEs should only occur in SACLs.
 */
bool sec_ace_resource(uint8_t type)
{
	return type == SEC_ACE_TYPE_SYSTEM_RESOURCE_ATTRIBUTE;
}
