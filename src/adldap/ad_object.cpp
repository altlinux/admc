/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "ad_object.h"

#include "ad_config.h"
#include "ad_display.h"
#include "ad_security.h"
#include "ad_utils.h"

#include "samba/ndr_security.h"

#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QMap>
#include <QString>
#include <algorithm>

AdObject::AdObject() {
}

void AdObject::load(const QString &dn_arg, const QHash<QString, QList<QByteArray>> &attributes_data_arg) {
    dn = dn_arg;
    attributes_data = attributes_data_arg;
}

QString AdObject::get_dn() const {
    return dn;
}

QHash<QString, QList<QByteArray>> AdObject::get_attributes_data() const {
    return attributes_data;
}

bool AdObject::is_empty() const {
    return attributes_data.isEmpty();
}

bool AdObject::contains(const QString &attribute) const {
    return attributes_data.contains(attribute);
}

QList<QString> AdObject::attributes() const {
    return attributes_data.keys();
}

QList<QByteArray> AdObject::get_values(const QString &attribute) const {
    if (contains(attribute)) {
        return attributes_data[attribute];
    } else {
        return QList<QByteArray>();
    }
}

QByteArray AdObject::get_value(const QString &attribute) const {
    const QList<QByteArray> values = get_values(attribute);

    if (!values.isEmpty()) {
        return values.first();
    } else {
        return QByteArray();
    }
}

QList<QString> AdObject::get_strings(const QString &attribute) const {
    const QList<QByteArray> values = get_values(attribute);

    QList<QString> strings;
    for (const auto &value : values) {
        const QString string = QString(value);
        strings.append(string);
    }

    return strings;
}

QString AdObject::get_string(const QString &attribute) const {
    const QList<QString> strings = get_strings(attribute);

    // NOTE: return last object class because that is the most derived one and is what's needed most of the time
    if (!strings.isEmpty()) {
        if (attribute == ATTRIBUTE_OBJECT_CLASS) {
            return strings.last();
        } else {
            return strings.first();
        }
    } else {
        return QString();
    }
}

QList<int> AdObject::get_ints(const QString &attribute) const {
    const QList<QString> strings = get_strings(attribute);

    QList<int> ints;
    for (const auto &string : strings) {
        const int int_value = string.toInt();
        ints.append(int_value);
    }

    return ints;
}

int AdObject::get_int(const QString &attribute) const {
    const QList<int> ints = get_ints(attribute);

    if (!ints.isEmpty()) {
        return ints.first();
    } else {
        return 0;
    }
}

QDateTime AdObject::get_datetime(const QString &attribute, const AdConfig *adconfig) const {
    const QString datetime_string = get_string(attribute);
    const QDateTime datetime = datetime_string_to_qdatetime(attribute, datetime_string, adconfig);

    return datetime;
}

QList<bool> AdObject::get_bools(const QString &attribute) const {
    const QList<QString> strings = get_strings(attribute);

    QList<bool> bools;
    for (const auto &string : strings) {
        const bool bool_value = ad_string_to_bool(string);
        bools.append(bool_value);
    }

    return bools;
}

bool AdObject::get_bool(const QString &attribute) const {
    const QList<bool> bools = get_bools(attribute);

    if (!bools.isEmpty()) {
        return bools.first();
    } else {
        return false;
    }
}

bool AdObject::get_system_flag(const SystemFlagsBit bit) const {
    if (contains(ATTRIBUTE_SYSTEM_FLAGS)) {
        const int system_flags_bits = get_int(ATTRIBUTE_SYSTEM_FLAGS);
        const bool is_set = bitmask_is_set(system_flags_bits, bit);

        return is_set;
    } else {
        return false;
    }
}

bool AdObject::get_account_option(AccountOption option, AdConfig *adconfig) const {
    switch (option) {
        case AccountOption_CantChangePassword: {
            const bool out = ad_security_get_user_cant_change_pass(this, adconfig);

            return out;
        }
        case AccountOption_PasswordExpired: {
            if (contains(ATTRIBUTE_PWD_LAST_SET)) {
                const QString pwdLastSet_value = get_string(ATTRIBUTE_PWD_LAST_SET);
                const bool expired = (pwdLastSet_value == AD_PWD_LAST_SET_EXPIRED);

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

// NOTE: "group type" is really only the last bit of the groupType attribute, yeah it's confusing
GroupType AdObject::get_group_type() const {
    const int group_type = get_int(ATTRIBUTE_GROUP_TYPE);

    const bool security_bit_set = ((group_type & GROUP_TYPE_BIT_SECURITY) != 0);

    if (security_bit_set) {
        return GroupType_Security;
    } else {
        return GroupType_Distribution;
    }
}

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

bool AdObject::is_class(const QString &object_class) const {
    const QString this_object_class = get_string(ATTRIBUTE_OBJECT_CLASS);
    const bool is_class = (this_object_class == object_class);

    return is_class;
}

QList<QString> AdObject::get_split_upn() const {
    const QString upn = get_string(ATTRIBUTE_USER_PRINCIPAL_NAME);
    const int split_index = upn.lastIndexOf('@');
    const QString prefix = upn.left(split_index);
    const QString suffix = upn.mid(split_index + 1);
    const QList<QString> upn_split = {prefix, suffix};

    return upn_split;
}

QString AdObject::get_upn_prefix() const {
    const QList<QString> upn_split = get_split_upn();

    return upn_split[0];
}

QString AdObject::get_upn_suffix() const {
    const QList<QString> upn_split = get_split_upn();

    return upn_split[1];
}

security_descriptor *AdObject::get_security_descriptor(TALLOC_CTX *mem_ctx_arg) const {
    TALLOC_CTX *mem_ctx = [&]() -> void * {
        if (mem_ctx_arg != nullptr) {
            return mem_ctx_arg;
        } else {
            return NULL;
        }
    }();

    const QByteArray sd_bytes = get_value(ATTRIBUTE_SECURITY_DESCRIPTOR);
    security_descriptor *out = security_descriptor_make_from_bytes(mem_ctx, sd_bytes);

    return out;
}
