/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2021-2026 BaseALT Ltd.
 * Copyright (C) 2021-2022 Dmitry Degtyarev
 * Copyright (C) 2025 Semyon Knyazev
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

#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QMap>
#include <QString>
#include <algorithm>

#include "ad_config.h"
#include "ad_display.h"
#include "ad_object.h"
#include "ad_security.h"
#include "ad_utils.h"
#include "samba/ndr_security.h"

AdObject::AdObject() {
    // Do nothing.
}

void AdObject::load(
    const QString &dn_arg,
    const QHash<QString, QList<QByteArray>> &attributes_data_arg)
{
    dn = dn_arg;
    attributes_data = attributes_data_arg;
}

/**
 * Get the distinguished name (DN) for the object.
 *
 * @return The distinguished object name as a string.
 */
QString AdObject::get_dn() const {
    return dn;
}

/**
 * Get object attributes data.
 *
 * @return The object attributes data as a hash table.
 */
QHash<QString, QList<QByteArray>> AdObject::get_attributes_data() const {
    return attributes_data;
}

/**
 * Check if the object attributes data is empty.
 *
 * @return True if it is empty, false otherwise.
 */
bool AdObject::is_empty() const {
    return attributes_data.isEmpty();
}

/**
 * Check if the object attributes contains the given attribute.
 *
 * @param attribute An attribute name to check.
 * @return True if the attributes data contains the given attribute,
 * false otherwise.
 */
bool AdObject::contains(const QString &attribute) const {
    return attributes_data.contains(attribute);
}

/**
 * Get the object attribute keys.
 *
 * @return A list of attribute keys.
 */
QList<QString> AdObject::attributes() const {
    return attributes_data.keys();
}

/**
 * Get the object attribute values.
 *
 * @param attribute An object attribute to query.
 * @return A list of object attribute values.
 */
QList<QByteArray> AdObject::get_values(const QString &attribute) const {
    if (contains(attribute)) {
        return attributes_data[attribute];
    } else {
        return QList<QByteArray>();
    }
}

/**
 * Get a the first value for a given object attribute.
 *
 * @param attribute An object attribute to query.
 * @return An object attribute value or an empty byte array.
 */
QByteArray AdObject::get_value(const QString &attribute) const {
    const QList<QByteArray> values = get_values(attribute);

    if (!values.isEmpty()) {
        return values.first();
    } else {
        return QByteArray();
    }
}

/**
 * Get a list of strings from the list of values for a given attribute.
 *
 * @param attribute An attribute to query.
 * @return A list of strings.
 */
QList<QString> AdObject::get_strings(const QString &attribute) const {
    const QList<QByteArray> values = get_values(attribute);

    QList<QString> strings;
    for (const auto &value : values) {
        const QString string = QString(value);
        strings.append(string);
    }

    return strings;
}

/**
 * Get the first or last value (depending on the object type) as a
 * string for a given attribute.
 *
 * @param attribute An attribute to query.
 * @return An attribute value converted to a string, or an empty string
 * if the attribute values list is empty.
 */
QString AdObject::get_string(const QString &attribute) const {
    const QList<QString> strings = get_strings(attribute);

    // NOTE: return last object class because that is the most derived one and
    // is what's needed most of the time
    if (! strings.isEmpty()) {
        if (attribute == ATTRIBUTE_OBJECT_CLASS) {
            return strings.last();
        } else {
            return strings.first();
        }
    } else {
        return QString();
    }
}

/**
 * Get a list of integers from the list of values for a given attribute.
 *
 * @param attribute An attribute to query.
 * @return A list of integers.
 */
QList<int> AdObject::get_ints(const QString &attribute) const {
    const QList<QString> strings = get_strings(attribute);

    QList<int> ints;
    for (const auto &string : strings) {
        const int int_value = string.toInt();
        ints.append(int_value);
    }

    return ints;
}

/**
 * Get the first or last value (depending on the object type) as an
 * integer for a given attribute.
 *
 * @param attribute An attribute to query.
 * @return An attribute value converted to an integer, or a zero
 * if the attribute values list is empty.
 *
 * XXX: How do we distinguish between a correct zero value and a zero
 * value returned in case of an empty list?
 */
int AdObject::get_int(const QString &attribute) const {
    const QList<int> ints = get_ints(attribute);
    return (! ints.isEmpty()) ? ints.first() : 0;
}

/**
 * Get a date/time value for a given attribute.
 *
 * @param attribute An attribute to query.
 * @param adconfig An AD config instance.
 * @return A date/time value.
 */
QDateTime AdObject::get_datetime(const QString &attribute,
                                 const AdConfig *adconfig) const {
    const QString datetime_string = get_string(attribute);
    return datetime_string_to_qdatetime(attribute, datetime_string, adconfig);
}

/**
 * Get a list of boolean values from the list of values for a given attribute.
 *
 * @param attribute An attribute to query.
 * @return A list of boolean values.
 */
QList<bool> AdObject::get_bools(const QString &attribute) const {
    const QList<QString> strings = get_strings(attribute);

    QList<bool> bools;
    for (const auto &string : strings) {
        const bool bool_value = ad_string_to_bool(string);
        bools.append(bool_value);
    }

    return bools;
}

/**
 * Get the first or last value (depending on the object type) as a
 * boolean for a given attribute.
 *
 * @param attribute An attribute to query.
 * @return An attribute value converted to a boolean, or "false" if the
 * attribute values list is empty.
 *
 * XXX: How do we distinguish between a correct "false" value and a
 * "false" value returned in case of an empty list?
 */
bool AdObject::get_bool(const QString &attribute) const {
    const QList<bool> bools = get_bools(attribute);
    return (! bools.isEmpty()) ? bools.first() : false;
}

/**
 * Get a system flag.
 *
 * @param bit A flag bit to check.
 * @return A boolean value representing the bit flag.
 */
bool AdObject::get_system_flag(const SystemFlagsBit bit) const {
    if (contains(ATTRIBUTE_SYSTEM_FLAGS)) {
        const int system_flags_bits = get_int(ATTRIBUTE_SYSTEM_FLAGS);
        const bool is_set = bitmask_is_set(system_flags_bits, bit);

        return is_set;
    } else {
        return false;
    }
}

/**
 * Get an account option.
 *
 * @param option An account option to get.
 * @param adconfig An AD config instance.
 * @return A boolean value for the options.
 */
bool AdObject::get_account_option(AccountOption option,
                                  AdConfig *adconfig) const {
    switch (option) {
        case AccountOption_CantChangePassword: {
            return ad_security_get_user_cant_change_pass(this, adconfig);
        }
        case AccountOption_PasswordExpired: {
            if (contains(ATTRIBUTE_PWD_LAST_SET)) {
                const QString pwdLastSet_value =
                    get_string(ATTRIBUTE_PWD_LAST_SET);
                const bool expired =
                    (pwdLastSet_value == AD_PWD_LAST_SET_EXPIRED);
                return expired;
            } else {
                return false;
            }
        }
        default: {
            // Account option is a UAC bit
            if (contains(ATTRIBUTE_USER_ACCOUNT_CONTROL)) {
                const int control = get_int(ATTRIBUTE_USER_ACCOUNT_CONTROL);
                const int bit = account_option_bit(option);
                const bool set = ((control & bit) != 0);
                return set;
            } else {
                return false;
            }
        }
    }
}

/**
 * Get the group type.
 *
 * @return The group type.
 *
 * NOTE: "group type" is really only the last bit of the groupType attribute,
 * yeah it's confusing -- Dmitry Degtyarev
 */
GroupType AdObject::get_group_type() const {
    const int group_type = get_int(ATTRIBUTE_GROUP_TYPE);
    const bool security_bit_set = ((group_type & GROUP_TYPE_BIT_SECURITY) != 0);
    return security_bit_set ? GroupType_Security : GroupType_Distribution;
}

/**
 * Get the group scope.
 *
 * @return The group scope.
 */
GroupScope AdObject::get_group_scope() const {
    const int group_type = get_int(ATTRIBUTE_GROUP_TYPE);

    for (int i = 0; i < GroupScope_COUNT; i++) {
        const GroupScope this_scope = (GroupScope) i;
        const int scope_bit = group_scope_bit(this_scope);

        if (bitmask_is_set(group_type, scope_bit)) {
            return this_scope;
        }
    }

    return GroupScope_Global;
}

/**
 * Check whether the object of a given class or not.
 *
 * @param object_class An object class name to compare this object class with.
 * @return True if the current object has the same class as the given class,
 * false otherwise.
 */
bool AdObject::is_class(const QString &object_class) const {
    const QString this_object_class = get_string(ATTRIBUTE_OBJECT_CLASS);
    return (this_object_class == object_class);
}

/**
 * Get the splitted user principal name (UPN) as a list of strings.
 *
 * @return The UP as a list of strings.
 */
QList<QString> AdObject::get_split_upn() const {
    const QString upn = get_string(ATTRIBUTE_USER_PRINCIPAL_NAME);
    const int split_index = upn.lastIndexOf('@');
    const QString prefix = upn.left(split_index);
    const QString suffix = upn.mid(split_index + 1);
    const QList<QString> upn_split = {prefix, suffix};

    return upn_split;
}

/**
 * Get the user principal name (UPN) prefix.
 *
 * @return The UPN prefix as a string.
 */
QString AdObject::get_upn_prefix() const {
    const QList<QString> upn_split = get_split_upn();

    return upn_split[0];
}

/**
 * Get the user principal name (UPN) suffix.
 *
 * @return The UPN suffix as a string.
 */
QString AdObject::get_upn_suffix() const {
    const QList<QString> upn_split = get_split_upn();

    return upn_split[1];
}

/**
 * Get the security descriptor.
 */
security_descriptor *AdObject::get_security_descriptor(
    TALLOC_CTX *mem_ctx_arg) const
{
    TALLOC_CTX *mem_ctx;
    if (mem_ctx_arg != nullptr) {
        mem_ctx = mem_ctx_arg;
    } else {
        mem_ctx = NULL;
    }

    const QByteArray sd_bytes = get_value(ATTRIBUTE_SECURITY_DESCRIPTOR);
    return security_descriptor_make_from_bytes(mem_ctx, sd_bytes);
}
