/*
 * GPGUI - Group Policy Editor GUI
 *
 * Copyright (C) 2020 BaseALT Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#if !defined(__GPTBACKEND_REGISTRY_H)
#define __GPTBACKEND_REGISTRY_H 1

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

const char *regtype2str(uint32_t &regtype);
uint32_t str2regtype(const char *regtype);

struct registry_entry {
    char *keyname;
    char *valuename;
    uint32_t regtype;
    char *data;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GPTBACKEND_REGISTRY_H */
