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

#include "status_bar.h"
#include "ad_interface.h"
#include "main_window.h"

StatusBar::StatusBar()
: QStatusBar()
{
    showMessage(tr("Ready"), 10 * 1000);

    connect(
        AD(), &AdInterface::ad_interface_login_complete,
        this, &StatusBar::on_ad_interface_login_complete);
    connect(
        AD(), &AdInterface::ad_interface_login_failed,
        this, &StatusBar::on_ad_interface_login_failed);

    connect(
        AD(), &AdInterface::load_children_failed,
        this, &StatusBar::on_load_children_failed);
    connect(
        AD(), &AdInterface::load_attributes_failed,
        this, &StatusBar::on_load_attributes_failed);

    connect(
        AD(), &AdInterface::create_entry_complete,
        this, &StatusBar::on_create_entry_complete);
    connect(
        AD(), &AdInterface::create_entry_failed,
        this, &StatusBar::on_create_entry_failed);

    connect(
        AD(), &AdInterface::set_attribute_complete,
        this, &StatusBar::on_set_attribute_complete);
    connect(
        AD(), &AdInterface::set_attribute_failed,
        this, &StatusBar::on_set_attribute_failed);

    connect(
        AD(), &AdInterface::create_entry_complete,
        this, &StatusBar::on_create_entry_complete);
    connect(
        AD(), &AdInterface::create_entry_failed,
        this, &StatusBar::on_create_entry_failed);
    
    connect(
        AD(), &AdInterface::move_user_complete,
        this, &StatusBar::on_move_user_complete);
    connect(
        AD(), &AdInterface::move_user_failed,
        this, &StatusBar::on_move_user_failed);

    connect(
        AD(), &AdInterface::add_user_to_group_complete,
        this, &StatusBar::on_add_user_to_group_complete);
    connect(
        AD(), &AdInterface::add_user_to_group_failed,
        this, &StatusBar::on_add_user_to_group_failed);

    connect(
        AD(), &AdInterface::delete_entry_complete,
        this, &StatusBar::on_delete_entry_complete);
    connect(
        AD(), &AdInterface::delete_entry_failed,
        this, &StatusBar::on_delete_entry_failed);

    connect(
        AD(), &AdInterface::rename_complete,
        this, &StatusBar::on_rename_complete);
    connect(
        AD(), &AdInterface::rename_failed,
        this, &StatusBar::on_rename_failed);
}

void StatusBar::on_ad_interface_login_complete(const QString &search_base, const QString &head_dn) {
    QString msg = QString("Logged in to \"%1\" with head dn at \"%2\"").arg(search_base, head_dn);

    showMessage(msg);
}

void StatusBar::on_ad_interface_login_failed(const QString &search_base, const QString &head_dn) {
    QString msg = QString("Failed to login to \"%1\" with head dn at \"%2\"").arg(search_base, head_dn);

    showMessage(msg);
}

void StatusBar::on_load_children_failed(const QString &dn, const QString &error_str) {
    QString msg = QString("Failed to load children of \"%1\". Error: \"%2\"").arg(dn, error_str);

    showMessage(msg);
}
void StatusBar::on_load_attributes_failed(const QString &dn, const QString &error_str) {
    QString msg = QString("Failed to load attributes of \"%1\". Error: \"%2\"").arg(dn, error_str);

    showMessage(msg);
}

void StatusBar::on_delete_entry_complete(const QString &dn) {
    QString msg = QString("Deleted entry \"%1\"").arg(dn);

    showMessage(msg);
}
void StatusBar::on_set_attribute_complete(const QString &dn, const QString &attribute, const QString &old_value, const QString &value) {
    QString msg = QString("Changed attribute \"%1\" of \"%2\" from \"%3\" to \"%4\"").arg(attribute, dn, old_value, value);

    showMessage(msg);
}
void StatusBar::on_create_entry_complete(const QString &dn, NewEntryType type) {
    QString type_str = new_entry_type_to_string[type];
    QString msg = QString("Created entry \"%1\" of type \"%2\"").arg(dn, type_str);

    showMessage(msg);
}
void StatusBar::on_move_user_complete(const QString &user_dn, const QString &container_dn, const QString &new_dn) {
    QString msg = QString("Moved entry \"%1\" to \"%2\"").arg(user_dn).arg(new_dn);

    showMessage(msg);
}
void StatusBar::on_add_user_to_group_complete(const QString &group_dn, const QString &user_dn) {
    QString msg = QString("Added user \"%1\" to group \"%2\"").arg(user_dn, group_dn);

    showMessage(msg);
}

// TODO: how to do translation of error messages coming from english-only lib???
// Probably will end up not using raw error strings anyway, just process error codes to generate localized string
void StatusBar::on_delete_entry_failed(const QString &dn, const QString &error_str) {
    QString msg = QString("Failed to delete entry \"%1\". Error: \"%2\"").arg(dn, error_str);

    showMessage(msg);
}
void StatusBar::on_set_attribute_failed(const QString &dn, const QString &attribute, const QString &old_value, const QString &value, const QString &error_str) {
    QString msg = QString("Failed to change attribute \"%1\" of entry \"%2\" from \"%3\" to \"%4\". Error: \"%5\"").arg(attribute, dn, old_value, value, error_str);

    showMessage(msg);
}
void StatusBar::on_create_entry_failed(const QString &dn, NewEntryType type, const QString &error_str) {
    QString type_str = new_entry_type_to_string[type];
    QString msg = QString("Failed to create entry \"%1\" of type \"%2\". Error: \"%3\"").arg(dn, type_str, error_str);

    showMessage(msg);
}
void StatusBar::on_move_user_failed(const QString &user_dn, const QString &container_dn, const QString &new_dn, const QString &error_str) {
    QString msg = QString("Failed to move user \"%1\". Error: \"%2\"").arg(user_dn, error_str);

    showMessage(msg);
}
void StatusBar::on_add_user_to_group_failed(const QString &group_dn, const QString &user_dn, const QString &error_str) {
    QString msg = QString("Failed to add user \"%1\" to group \"%2\". Error: \"%3\"").arg(user_dn, group_dn, error_str);

    showMessage(msg);
}

void StatusBar::on_rename_complete(const QString &dn, const QString &new_name, const QString &new_dn) {
    QString msg = QString("Renamed \"%1\" to \"%2\"").arg(dn, new_name);

    showMessage(msg);
}

void StatusBar::on_rename_failed(const QString &dn, const QString &new_name, const QString &new_dn, const QString &error_str) {
    QString msg = QString("Failed to rename \"%1\" to \"%2\". Error: \"%3\"").arg(dn, new_name, error_str);

    showMessage(msg);
}