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
#include <QHash>

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

typedef QMap<QString, QList<QString>> Attributes;

class AdInterface final : public QObject {
Q_OBJECT

public:
    explicit AdInterface(QObject *parent);
    ~AdInterface();

    void ad_interface_login(const QString &base, const QString &head);
    QString get_error_str();
    QString get_search_base();
    QString get_uri();

    QList<QString> load_children(const QString &dn);
    QList<QString> search(const QString &filter);

    Attributes get_attributes(const QString &dn);
    QList<QString> get_attribute_multi(const QString &dn, const QString &attribute);
    QString get_attribute(const QString &dn, const QString &attribute);
    bool attribute_value_exists(const QString &dn, const QString &attribute, const QString &value);

    bool set_attribute(const QString &dn, const QString &attribute, const QString &value);
    bool create_entry(const QString &name, const QString &dn, NewEntryType type);
    void delete_entry(const QString &dn);
    void move(const QString &dn, const QString &new_container);
    void add_user_to_group(const QString &group_dn, const QString &user_dn);
    void group_remove_user(const QString &group_dn, const QString &user_dn);
    void rename(const QString &dn, const QString &new_name);

    bool is_user(const QString &dn);
    bool is_group(const QString &dn);
    bool is_container(const QString &dn);
    bool is_ou(const QString &dn);
    bool is_policy(const QString &dn);
    bool is_container_like(const QString &dn);

    bool can_drop_entry(const QString &dn, const QString &target_dn);
    void drop_entry(const QString &dn, const QString &target_dn);

    void command(QStringList args);

signals:
    void modified();
    void logged_in();
    void message(const QString &msg);

private:
    adldap::AdConnection *connection = nullptr;
    QHash<QString, Attributes> attributes_cache;

    QMap<QString, QList<QString>> load_attributes(const QString &dn);

}; 

// Convenience function to get AdInterface from qApp instance
AdInterface *AD();

QString filter_EQUALS(const QString &attribute, const QString &value);
QString filter_AND(const QString &a, const QString &b);
QString filter_OR(const QString &a, const QString &b);
QString filter_NOT(const QString &a);

#endif /* AD_INTERFACE_H */
