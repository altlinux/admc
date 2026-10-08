/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
 * Copyright (C) 2026 Artyom V. Poptsov
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

#ifndef AD_LDAP_UTILS_H
#define AD_LDAP_UTILS_H

#include <ldap.h>

LDAPDN ad_ldap_string_to_dn(const QString &dn_string);
QString ad_ldap_dn_to_string(LDAPDN dn);
bool ad_ldap_dn_replace_value(LDAPDN dn,
                              const uint32_t &rdn_index,
                              const QString &new_value);
QString ad_ldap_dn_string_replace_value(const QString &input,
                                        const uint32_t &rdn_index,
                                        const QString &new_value);
size_t ad_ldap_dn_rdn_count(const LDAPDN dn);

#endif /* AD_LDAP_UTILS_H */
