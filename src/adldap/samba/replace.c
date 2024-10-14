/*
   Unix SMB/CIFS implementation.
   replacement routines for broken systems
   Copyright (C) Andrew Tridgell 1992-1998
   Copyright (C) Jelmer Vernooij 2005-2008
   Copyright (C) Matthieu Patou  2010

     ** NOTE! The following LGPL license applies to the replace
     ** library. This does NOT imply that all of Samba is released
     ** under the LGPL

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see <http://www.gnu.org/licenses/>.
*/

/*
 * This file is a copy of private samba sources. Parts of it
 * were removed or edited.
 */

#include "replace.h"

#include <errno.h>
#include <string.h>

/*
 * Like strncpy but does not 0 fill the buffer and always null
 * terminates. bufsize is the size of the destination buffer.
 * Returns the length of s.
 */
size_t rep_strlcpy(char *d, const char *s, size_t bufsize)
{
    size_t len = strlen(s);
    size_t ret = len;

    if (bufsize <= 0) {
        return 0;
    }
    if (len >= bufsize) {
        len = bufsize - 1;
    }
    memcpy(d, s, len);
    d[len] = 0;
    return ret;
}

#ifndef RSIZE_MAX
# define RSIZE_MAX (SIZE_MAX >> 1)
#endif

int rep_memset_s(void *dest, size_t destsz, int ch, size_t count)
{
    if (dest == NULL) {
        return EINVAL;
    }

    if (destsz > RSIZE_MAX ||
        count > RSIZE_MAX ||
        count > destsz) {
        return ERANGE;
    }

    memset(dest, ch, count);

    return 0;
}
