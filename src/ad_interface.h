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

// Interface between the GUI and AdConnection
// Stores attributes cache of entries
// Attributes cache is expanded as more entries are loaded and
// is updated on entry changes
// Emits various signals for AD operation successes/failures

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
    QString get_error_str();

    QList<QString> load_children(const QString &dn);

    // These functions load attributes if haven't loaded yet
    QMap<QString, QList<QString>> get_attributes(const QString &dn);
    QList<QString> get_attribute_multi(const QString &dn, const QString &attribute);
    QString get_attribute(const QString &dn, const QString &attribute);
    bool attribute_value_exists(const QString &dn, const QString &attribute, const QString &value);
    bool is_user(const QString &dn);
    bool is_group(const QString &dn);
    bool is_container(const QString &dn);
    bool is_ou(const QString &dn);
    bool is_policy(const QString &dn);
    bool is_container_like(const QString &dn);

    bool set_attribute(const QString &dn, const QString &attribute, const QString &value);
    void create_entry(const QString &name, const QString &dn, NewEntryType type);
    void delete_entry(const QString &dn);
    void move(const QString &dn, const QString &new_container);
    void add_user_to_group(const QString &group_dn, const QString &user_dn);
    void rename(const QString &dn, const QString &new_name);

    bool can_drop_entry(const QString &dn, const QString &target_dn);
    void drop_entry(const QString &dn, const QString &target_dn);

signals:
    void ad_interface_login_complete(const QString &base, const QString &head);
    void ad_interface_login_failed(const QString &base, const QString &head);

    void load_children_failed(const QString &dn, const QString &error_str);

    void load_attributes_complete(const QString &dn);
    void load_attributes_failed(const QString &dn, const QString &error_str);

    void delete_entry_complete(const QString &dn);
    void delete_entry_failed(const QString &dn, const QString &error_str);

    void set_attribute_complete(const QString &dn, const QString &attribute, const QString &old_value, const QString &value);
    void set_attribute_failed(const QString &dn, const QString &attribute, const QString &old_value, const QString &value, const QString &error_str);

    void create_entry_complete(const QString &dn, NewEntryType type);
    void create_entry_failed(const QString &dn, NewEntryType type, const QString &error_str);

    void move_complete(const QString &dn, const QString &new_container, const QString &new_dn);
    void move_failed(const QString &dn, const QString &new_container, const QString &new_dn, const QString &error_str);
    
    void add_user_to_group_complete(const QString &group_dn, const QString &user_dn);
    void add_user_to_group_failed(const QString &group_dn, const QString &user_dn, const QString &error_str);

    void rename_complete(const QString &dn, const QString &new_name, const QString &new_dn);
    void rename_failed(const QString &dn, const QString &new_name, const QString &new_dn, const QString &error_str);

    void dn_changed(const QString &dn, const QString &new_dn);
    void attributes_changed(const QString &dn);

private:
    adldap::AdConnection *connection = nullptr;
    QMap<QString, QMap<QString, QList<QString>>> attributes_map;
    QSet<QString> attributes_loaded_set;

    void load_attributes(const QString &dn);
    bool attributes_loaded(const QString &dn);
    void update_cache(const QString &old_dn, const QString &new_dn);

}; 

// Convenience function to get AdInterface from qApp instance
AdInterface *AD();

#endif /* AD_INTERFACE_H */
