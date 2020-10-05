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

typedef QHash<QString, QList<QByteArray>> AdObjectData;

class AdObject {

public:
    AdObject();
    AdObject(const AdObjectData &data_arg);

    bool is_empty() const;
    bool contains(const QString &attribute) const;
    QList<QString> keys() const;

    QList<QByteArray> get_values(const QString &attribute) const;
    QByteArray get_value(const QString &attribute) const;

    QList<QString> get_strings(const QString &attribute) const;
    QString get_string(const QString &attribute) const;

    int get_int(const QString &attribute) const;
    bool get_system_flag(const SystemFlagsBit bit) const;
    bool get_account_option(AccountOption option) const;
    GroupScope get_group_scope() const;
    GroupType get_group_type() const;

    bool is_class(const QString &object_class) const;
    bool is_user() const;
    bool is_group() const;
    bool is_container() const;
    bool is_ou() const;
    bool is_policy() const;
    bool is_computer() const;

    QIcon get_icon() const;

private:
    AdObjectData data;
};

#endif /* ATTRIBUTES_H */
