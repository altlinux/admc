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

#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include "ad_enums.h"

#include <QHash>
#include <QString>
#include <QList>
#include <QByteArray>
#include <QIcon>
#include <QDateTime>

// This object is returned as a result of some AdInterface
// functions. It stores AD object's attributes and provides
// read-only access to them. Modifying attributes can be done
// through AdInterface.
// Note that this object is loaded with data once and not updated
// afterwards so it WILL become out of date after any
// AD modification. Therefore, do not keep it around for too long.

typedef QHash<QString, QList<QByteArray>> AdObjectAttributes;

class AdObject {

public:
    AdObject();
    AdObject(const QString &dn_arg, const AdObjectAttributes &attributes_data_arg);

    QString get_dn() const;
    bool is_empty() const;
    bool contains(const QString &attribute) const;
    QList<QString> attributes() const;

    QList<QByteArray> get_values(const QString &attribute) const;
    QByteArray get_value(const QString &attribute) const;

    QList<QString> get_strings(const QString &attribute) const;
    QString get_string(const QString &attribute) const;

    int get_int(const QString &attribute) const;
    QList<int> get_ints(const QString &attribute) const;

    QList<bool> get_bools(const QString &attribute) const;
    bool get_bool(const QString &attribute) const;

    QDateTime get_datetime(const QString &attribute) const;

    bool get_system_flag(const SystemFlagsBit bit) const;

    bool get_account_option(AccountOption option) const;

    GroupScope get_group_scope() const;
    GroupType get_group_type() const;

    // NOTE: this compares for the most derived class, so each object only maps to one class. For example computers have objectClass values of both "user" and "computer" but "computer" is more derived so they are only computers and not users.
    bool is_class(const QString &object_class) const;

    QIcon get_icon() const;

private:
    QString dn;
    AdObjectAttributes attributes_data;
};

#endif /* ATTRIBUTES_H */
