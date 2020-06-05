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

#ifndef AD_INTERFACE_H
#define AD_INTERFACE_H

#include <QObject>
#include <QList>
#include <QString>
#include <QMap>
#include <QSet>

class AdConnection;

namespace adldap
{
    class AdConnection;
};

// Interface functions to convert from raw char** active directory returns to Qt containers
// Also can load fake data if program is run with "fake" option

enum NewEntryType {
    User,
    Computer,
    OU,
    Group,
    COUNT
};

const QMap<NewEntryType, QString> new_entry_type_to_string = {
    {NewEntryType::User, "User"},
    {NewEntryType::Computer, "Computer"},
    {NewEntryType::OU, "Organization Unit"},
    {NewEntryType::Group, "Group"},
};

QString extract_name_from_dn(const QString &dn);
QString extract_parent_dn_from_dn(const QString &dn);

class AdInterface final : public QObject {
Q_OBJECT

public:
    explicit AdInterface(QObject *parent);
    ~AdInterface();

    void ad_interface_login(const QString &base, const QString &head);

    QList<QString> load_children(const QString &dn);
    QMap<QString, QList<QString>> get_attributes(const QString &dn);
    QList<QString> get_attribute_multi(const QString &dn, const QString &attribute);
    QString get_attribute(const QString &dn, const QString &attribute);
    bool attribute_value_exists(const QString &dn, const QString &attribute, const QString &value);

    bool set_attribute(const QString &dn, const QString &attribute, const QString &value);
    bool create_entry(const QString &name, const QString &dn, NewEntryType type);
    void delete_entry(const QString &dn);
    void move_user(const QString &user_dn, const QString &container_dn);
    void add_user_to_group(const QString &group_dn, const QString &user_dn);
    QString get_error_str();

signals:
    void ad_interface_login_complete(const QString &base, const QString &head);
    void ad_interface_login_failed(const QString &base, const QString &head);

    void load_children_failed(const QString &dn, const QString &error_str);
    void load_attributes_complete(const QString &dn);
    void load_attributes_failed(const QString &dn, const QString &error_str);

    void delete_entry_complete(const QString &dn);
    void set_attribute_complete(const QString &dn, const QString &attribute, const QString &old_value, const QString &value);
    void create_entry_complete(const QString &dn, NewEntryType type);
    void move_user_complete(const QString &user_dn, const QString &container_dn, const QString &new_dn);
    void add_user_to_group_complete(const QString &group_dn, const QString &user_dn);

    void delete_entry_failed(const QString &dn, const QString &error_str);
    void set_attribute_failed(const QString &dn, const QString &attribute, const QString &old_value, const QString &value, const QString &error_str);
    void create_entry_failed(const QString &dn, NewEntryType type, const QString &error_str);
    void move_user_failed(const QString &user_dn, const QString &container_dn, const QString &new_dn, const QString &error_str);
    void add_user_to_group_failed(const QString &group_dn, const QString &user_dn, const QString &error_str);

private:
    adldap::AdConnection *connection = nullptr;
    QMap<QString, QMap<QString, QList<QString>>> attributes_map;
    QSet<QString> attributes_loaded;

    void load_attributes(const QString &dn);
    void reload_attributes_of_entry_groups(const QString &dn);

}; 

#endif /* AD_INTERFACE_H */
