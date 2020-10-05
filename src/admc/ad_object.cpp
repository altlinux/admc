/*
 * ADMC - AD Management Center
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

#include "ad_object.h"
#include "ad_interface.h"
#include "utils.h"

AdObject::AdObject()
{

}

AdObject::AdObject(const QString &dn_arg, const AdObjectAttributes &attributes_data_arg)
: dn(dn_arg)
, attributes_data(attributes_data_arg)
{

}

QString AdObject::get_dn() const {
    return dn;
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
        return values[0];
    } else {
        return QByteArray();
    }
}

QList<QString> AdObject::get_strings(const QString &attribute) const {
    const QList<QByteArray> values = get_values(attribute);
    const QList<QString> strings = byte_arrays_to_strings(values);

    return strings;
}

QString AdObject::get_string(const QString &attribute) const {
    const QByteArray bytes = get_value(attribute);
    const QString string = QString::fromUtf8(bytes);

    return string;
}

QList<int> AdObject::get_ints(const QString &attribute) const {
    const QList<QString> strings = get_strings(attribute);

    QList<int> ints;
    for (const auto string : strings) {
        const int value_int = string.toInt();
        ints.append(value_int);
    }

    return ints;
}

QDateTime AdObject::get_datetime(const QString &attribute) const {
    const QString datetime_string = get_value(attribute);

    return datetime_string_to_qdatetime(attribute, datetime_string);
}

int AdObject::get_int(const QString &attribute) const {
    const QString value_raw = get_value(attribute);
    const int value = value_raw.toInt();

    return value;
}

bool AdObject::get_system_flag(const SystemFlagsBit bit) const {
    const int system_flags_bits = get_int(ATTRIBUTE_SYSTEM_FLAGS);
    const bool is_set = bit_is_set(system_flags_bits, bit);

    return is_set;
}

bool AdObject::get_account_option(AccountOption option) const {
    switch (option) {
        case AccountOption_PasswordExpired: {
            const QString pwdLastSet_value = get_string(ATTRIBUTE_PWD_LAST_SET);
            const bool expired = (pwdLastSet_value == AD_PWD_LAST_SET_EXPIRED);

            return expired;
        }
        default: {
            // Account option is a UAC bit
            if (contains(ATTRIBUTE_USER_ACCOUNT_CONTROL)) {
                return false;
            } else {
                const int control = get_int(ATTRIBUTE_USER_ACCOUNT_CONTROL);
                const int bit = account_option_bit(option);

                const bool set = ((control & bit) != 0);

                return set;
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

        if (bit_is_set(group_type, scope_bit)) {
            return this_scope;
        }
    }

    return GroupScope_Global;
}

bool AdObject::is_class(const QString &object_class) const {
    const QList<QByteArray> object_classes = attributes_data[ATTRIBUTE_OBJECT_CLASS];
    const bool is_class = object_classes.contains(object_class.toUtf8());

    return is_class;
}

bool AdObject::is_user() const {
    return is_class(CLASS_USER) && !is_class(CLASS_COMPUTER);
}

bool AdObject::is_group() const {
    return is_class(CLASS_GROUP);
}

bool AdObject::is_container() const {
    return is_class(CLASS_CONTAINER);
}

bool AdObject::is_ou() const {
    return is_class(CLASS_OU);
}

bool AdObject::is_policy() const {
    return is_class(CLASS_GP_CONTAINER);
}

bool AdObject::is_computer() const {
    return is_class(CLASS_COMPUTER);
}

QIcon AdObject::get_icon() const {
    // TODO: change to custom, good icons, add those icons to installation?
    // TODO: are there cases where an object can have multiple icons due to multiple objectClasses and one of them needs to be prioritized?
    static const QMap<QByteArray, QString> class_to_icon = {
        {QByteArray(CLASS_GP_CONTAINER), "x-office-address-book"},
        {QByteArray(CLASS_CONTAINER), "folder"},
        {QByteArray(CLASS_OU), "network-workgroup"},
        {QByteArray(CLASS_PERSON), "avatar-default"},
        {QByteArray(CLASS_GROUP), "application-x-smb-workgroup"},
        {QByteArray(CLASS_BUILTIN_DOMAIN), "emblem-system"},
    };

    const QList<QByteArray> object_classes = attributes_data[ATTRIBUTE_OBJECT_CLASS];
    const QString icon_name =
    [object_classes]() {
        for (auto c : class_to_icon.keys()) {
            if (object_classes.contains(c)) {
                return class_to_icon[c];
            }
        }

        return QString("dialog-question");
    }();

    const QIcon icon = QIcon::fromTheme(icon_name);

    return icon;
}
