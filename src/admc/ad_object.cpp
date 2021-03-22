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
#include "ad_utils.h"
#include "utils.h"

#include <algorithm>
#include <QMap>

AdObject::AdObject()
{

}

void AdObject::load(const QString &dn_arg, const AdObjectAttributes &attributes_data_arg) {
    dn = dn_arg;
    attributes_data = attributes_data_arg;
}

QString AdObject::get_dn() const {
    return dn;
}

AdObjectAttributes AdObject::get_attributes_data() const {
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

QDateTime AdObject::get_datetime(const QString &attribute) const {
    const QString datetime_string = get_string(attribute);
    const QDateTime datetime = datetime_string_to_qdatetime(attribute, datetime_string);

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
        const bool is_set = bit_is_set(system_flags_bits, bit);

        return is_set;
    } else {
        return false;
    }
}

bool AdObject::get_account_option(AccountOption option) const {
    switch (option) {
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

        if (bit_is_set(group_type, scope_bit)) {
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

QIcon AdObject::get_icon() const {
    // TODO: change to custom, good icons, add those icons to installation?
    static const QMap<QString, QString> class_to_icon = {
        {CLASS_DOMAIN, "network-server"},
        {CLASS_CONTAINER, "folder"},
        {CLASS_OU, "folder-documents"},
        {CLASS_GROUP, "application-x-smb-workgroup"},
        {CLASS_PERSON, "avatar-default"},
        {CLASS_COMPUTER, "computer"},
        {CLASS_GP_CONTAINER, "folder-templates"},

        // Some custom icons for one-off objects
        {"builtinDomain", "emblem-system"},
        {"configuration", "emblem-system"},
        {"lostAndFound", "emblem-system"},
    };

    // Iterate over object classes in reverse, starting from most inherited class
    QList<QString> object_classes = get_strings(ATTRIBUTE_OBJECT_CLASS);
    std::reverse(object_classes.begin(), object_classes.end());

    const QString icon_name =
    [object_classes]() -> QString {
        for (auto object_class : object_classes) {
            if (class_to_icon.contains(object_class)) {
                return class_to_icon[object_class];
            }
        }

        return "dialog-question";
    }();

    const QIcon icon = QIcon::fromTheme(icon_name);

    return icon;
}
