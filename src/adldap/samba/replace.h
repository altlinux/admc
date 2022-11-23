/*
   Unix SMB/CIFS implementation.

   macros to go along with the lib/replace/ portability layer code

   Copyright (C) Andrew Tridgell 2005
   Copyright (C) Jelmer Vernooij 2006-2008
   Copyright (C) Jeremy Allison 2007.

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

#ifndef _LIBREPLACE_REPLACE_H
#define _LIBREPLACE_REPLACE_H

#include <stddef.h>
#include <inttypes.h>
#include <ctype.h>

#ifndef __PRI64_PREFIX
# if __WORDSIZE == 64 && ! defined __APPLE__
#  define __PRI64_PREFIX    "l"
# else
#  define __PRI64_PREFIX    "ll"
# endif
#endif

#ifndef PRIu8
# define PRIu8      "u"
#endif
#ifndef PRIu16
# define PRIu16     "u"
#endif
#ifndef PRIu32
# define PRIu32     "u"
#endif
#ifndef PRIu64
# define PRIu64     __PRI64_PREFIX "u"
#endif

#ifndef HAVE_MEMSET_S
#define memset_s rep_memset_s
int rep_memset_s(void *dest, size_t destsz, int ch, size_t count);
#endif

#define strlcpy rep_strlcpy
size_t rep_strlcpy(char *d, const char *s, size_t bufsize);

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

/**
 * Work out how many elements there are in a static array.
 */
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

/**
 * Zero a structure.
 */
#define ZERO_STRUCT(x) memset_s((char *)&(x), sizeof(x), 0, sizeof(x))

/**
 * Zero a structure given a pointer to the structure.
 */
#define ZERO_STRUCTP(x) do { \
    if ((x) != NULL) { \
        memset_s((char *)(x), sizeof(*(x)), 0, sizeof(*(x))); \
    } \
} while(0)

/**
 * Zero a structure given a pointer to the structure - no zero check
 */
#define ZERO_STRUCTPN(x) memset_s((char *)(x), sizeof(*(x)), 0, sizeof(*(x)))

#endif /* _LIBREPLACE_REPLACE_H */
