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

/**
 * @file ad_ldap_utils.cpp
 * @brief ADMC LDAP utilities.
 */

#include <ldap.h>

/**
 * Convert a string to an LDAP distinguished name.
 *
 * @param dn A distinguished name string to convert.
 * @return An LDAPDN instance or nullptr if the name is invalid.
 * A caller must free this object after usage with "ldap_dnfree".
 */
LDAPDN ad_ldap_string_to_dn(const QString &dn_string) {
    LDAPDN dn = nullptr;
    int rc = ldap_str2dn(dn_string.toUtf8().toStdString().c_str(),
                         &dn,
                         LDAP_DN_FORMAT_LDAPV3);
    if (rc != LDAP_SUCCESS) {
        return nullptr;
    }

    return dn;
}

/**
 * Convert an LDAP distinguished name to a Qt string.
 *
 * @param dn A LDAP distinguished name to convert.
 * @return A Qt string or nullptr on serialization errors.
 */
QString ad_ldap_dn_to_string(LDAPDN dn) {
    char* output = nullptr;
    int rc = ldap_dn2str(dn, &output, LDAP_DN_FORMAT_LDAPV3);
    if (rc != LDAP_SUCCESS) {
        return nullptr;
    }
    QString result(output);
    ldap_memfree(output);
    return result;
}


/**
 * Get the number of relative distinguished names (RDNs) in the provided
 * distinguished name (DN.)
 *
 * @param dn A distinguished name (DN) to process.
 * @return The number of RDNs in the given DN.
 */
size_t ad_ldap_dn_rdn_count(const LDAPDN dn) {
    size_t count = 0;

    if (dn == nullptr) {
        return 0;
    }

    while (dn[count] != nullptr) {
        count++;
    }

    return count;
}

/**
 * Replace an LDAP RDN value with a new value.
 *
 * Asserts that @a dn is not nullptr.
 *
 * @param dn A DN object to modify.
 * @param rdn_index The RDN value index.
 * @param new_value The new value to set.
 * @return true on success, false on errors.
 */
bool ad_ldap_dn_replace_value(LDAPDN dn,
                              const uint32_t &rdn_index,
                              const QString &new_value) {
    assert(dn != nullptr);

    size_t rdn_count = ad_ldap_dn_rdn_count(dn);
    if (rdn_index >= rdn_count) {
        return false;
    }

    LDAPRDN rdn = dn[rdn_index];
    if (! rdn) {
        return false;
    }

    LDAPAVA *ava = rdn[0];
    if (! ava) {
        return false;
    }

    if (! ava->la_value.bv_val) {
        return false;
    }

    const size_t NEW_SIZE = new_value.toUtf8().size() + 1;
    char* new_bv_val = static_cast<char*>(ber_memalloc(NEW_SIZE));
    if (! new_bv_val) {
        return false;
    }

    ber_memfree(ava->la_value.bv_val);

    std::string std_string = new_value.toUtf8().toStdString();
    const char* new_value_str = std_string.c_str();
    int len = strlen(new_value_str);

    ava->la_value.bv_len = len;
    ava->la_value.bv_val = new_bv_val;
    std::memcpy(ava->la_value.bv_val, new_value_str, len);
    ava->la_value.bv_val[len] = '\0';

    return true;
}

QString ad_ldap_dn_string_replace_value(const QString& input,
                                        const uint32_t &rdn_index,
                                        const QString& new_value) {
    LDAPDN dn = ad_ldap_string_to_dn(input);
    if (dn == nullptr) {
        throw std::runtime_error("Invalid LDAP DN");
    }

    if (! ad_ldap_dn_replace_value(dn, rdn_index, new_value)) {
        ldap_dnfree(dn);
        throw std::runtime_error("Could not modify an LDAP RDN value");
    }

    QString result = ad_ldap_dn_to_string(dn);
    if (result == nullptr) {
        ldap_dnfree(dn);
        throw std::runtime_error("Could not convert a LDAP DN to a string");
    }

    ldap_dnfree(dn);

    return result;
}
