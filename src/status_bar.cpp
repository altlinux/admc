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
#include "settings.h"

#include <QStatusBar>
#include <QTextEdit>
#include <QAction>

Status::Status(QStatusBar *status_bar_, QTextEdit *status_log_, QObject *parent)
: QObject(parent)
{
    status_bar = status_bar_;
    status_log = status_log_;

    showMessage(tr("Ready"), 10 * 1000);

    connect(
        AD(), &AdInterface::ad_interface_login_complete,
        this, &Status::on_ad_interface_login_complete);
    connect(
        AD(), &AdInterface::ad_interface_login_failed,
        this, &Status::on_ad_interface_login_failed);

    connect(
        AD(), &AdInterface::load_children_failed,
        this, &Status::on_load_children_failed);
    connect(
        AD(), &AdInterface::load_attributes_failed,
        this, &Status::on_load_attributes_failed);

    connect(
        AD(), &AdInterface::create_entry_complete,
        this, &Status::on_create_entry_complete);
    connect(
        AD(), &AdInterface::create_entry_failed,
        this, &Status::on_create_entry_failed);

    connect(
        AD(), &AdInterface::set_attribute_complete,
        this, &Status::on_set_attribute_complete);
    connect(
        AD(), &AdInterface::set_attribute_failed,
        this, &Status::on_set_attribute_failed);

    connect(
        AD(), &AdInterface::create_entry_complete,
        this, &Status::on_create_entry_complete);
    connect(
        AD(), &AdInterface::create_entry_failed,
        this, &Status::on_create_entry_failed);
    
    connect(
        AD(), &AdInterface::move_complete,
        this, &Status::on_move_complete);
    connect(
        AD(), &AdInterface::move_failed,
        this, &Status::on_move_failed);

    connect(
        AD(), &AdInterface::add_user_to_group_complete,
        this, &Status::on_add_user_to_group_complete);
    connect(
        AD(), &AdInterface::add_user_to_group_failed,
        this, &Status::on_add_user_to_group_failed);

    connect(
        AD(), &AdInterface::delete_entry_complete,
        this, &Status::on_delete_entry_complete);
    connect(
        AD(), &AdInterface::delete_entry_failed,
        this, &Status::on_delete_entry_failed);

    connect(
        AD(), &AdInterface::rename_complete,
        this, &Status::on_rename_complete);
    connect(
        AD(), &AdInterface::rename_failed,
        this, &Status::on_rename_failed);

    connect(
        SETTINGS()->toggle_show_status_log, &QAction::toggled,
        this, &Status::on_toggle_show_status_log);

    // Load "show status log" setting
    const bool show_status_log_checked = SETTINGS()->toggle_show_status_log->isChecked();
    on_toggle_show_status_log(show_status_log_checked);
}

void Status::on_ad_interface_login_complete(const QString &search_base, const QString &head_dn) {
    QString msg = QString("Logged in to \"%1\" with head dn at \"%2\"").arg(search_base, head_dn);

    showMessage(msg);
}

void Status::on_ad_interface_login_failed(const QString &search_base, const QString &head_dn) {
    QString msg = QString("Failed to login to \"%1\" with head dn at \"%2\"").arg(search_base, head_dn);

    showMessage(msg);
}

void Status::on_load_children_failed(const QString &dn, const QString &error_str) {
    QString msg = QString("Failed to load children of \"%1\". Error: \"%2\"").arg(dn, error_str);

    showMessage(msg);
}
void Status::on_load_attributes_failed(const QString &dn, const QString &error_str) {
    QString msg = QString("Failed to load attributes of \"%1\". Error: \"%2\"").arg(dn, error_str);

    showMessage(msg);
}

void Status::on_delete_entry_complete(const QString &dn) {
    QString msg = QString("Deleted entry \"%1\"").arg(dn);

    showMessage(msg);
}
void Status::on_set_attribute_complete(const QString &dn, const QString &attribute, const QString &old_value, const QString &value) {
    QString msg = QString("Changed attribute \"%1\" of \"%2\" from \"%3\" to \"%4\"").arg(attribute, dn, old_value, value);

    showMessage(msg);
}
void Status::on_create_entry_complete(const QString &dn, NewEntryType type) {
    QString type_str = new_entry_type_to_string[type];
    QString msg = QString("Created entry \"%1\" of type \"%2\"").arg(dn, type_str);

    showMessage(msg);
}
void Status::on_move_complete(const QString &dn, const QString &new_container, const QString &new_dn) {
    QString msg = QString("Moved \"%1\" to \"%2\"").arg(dn).arg(new_container);

    showMessage(msg);
}
void Status::on_add_user_to_group_complete(const QString &group_dn, const QString &user_dn) {
    QString msg = QString("Added user \"%1\" to group \"%2\"").arg(user_dn, group_dn);

    showMessage(msg);
}

// TODO: how to do translation of error messages coming from english-only lib???
// Probably will end up not using raw error strings anyway, just process error codes to generate localized string
void Status::on_delete_entry_failed(const QString &dn, const QString &error_str) {
    QString msg = QString("Failed to delete entry \"%1\". Error: \"%2\"").arg(dn, error_str);

    showMessage(msg);
}
void Status::on_set_attribute_failed(const QString &dn, const QString &attribute, const QString &old_value, const QString &value, const QString &error_str) {
    QString msg = QString("Failed to change attribute \"%1\" of entry \"%2\" from \"%3\" to \"%4\". Error: \"%5\"").arg(attribute, dn, old_value, value, error_str);

    showMessage(msg);
}
void Status::on_create_entry_failed(const QString &dn, NewEntryType type, const QString &error_str) {
    QString type_str = new_entry_type_to_string[type];
    QString msg = QString("Failed to create entry \"%1\" of type \"%2\". Error: \"%3\"").arg(dn, type_str, error_str);

    showMessage(msg);
}
void Status::on_move_failed(const QString &dn, const QString &new_container, const QString &new_dn, const QString &error_str) {
    QString msg = QString("Failed to move \"%1\" to \"%2\". Error: \"%3\"").arg(dn, new_container, error_str);

    showMessage(msg);
}
void Status::on_add_user_to_group_failed(const QString &group_dn, const QString &user_dn, const QString &error_str) {
    QString msg = QString("Failed to add user \"%1\" to group \"%2\". Error: \"%3\"").arg(user_dn, group_dn, error_str);

    showMessage(msg);
}

void Status::on_rename_complete(const QString &dn, const QString &new_name, const QString &new_dn) {
    QString msg = QString("Renamed \"%1\" to \"%2\"").arg(dn, new_name);

    showMessage(msg);
}

void Status::on_rename_failed(const QString &dn, const QString &new_name, const QString &new_dn, const QString &error_str) {
    QString msg = QString("Failed to rename \"%1\" to \"%2\". Error: \"%3\"").arg(dn, new_name, error_str);

    showMessage(msg);
}

void Status::on_toggle_show_status_log(bool checked) {
    if (checked) {
        status_log->setVisible(true); 
    } else {
        status_log->setVisible(false); 
    }
}

void Status::showMessage(const QString &msg, int status_bar_timeout) {
    status_bar->showMessage(msg, status_bar_timeout);

    const QColor original_color = status_log->textColor();
    QColor color = original_color;
    if (msg.contains("Failed")) {
        color = Qt::red;
    } else {
        color = Qt::darkGreen;
    }

    status_log->setTextColor(color);
    status_log->append(msg);
    status_log->setTextColor(original_color);

    // Scroll text edit to the newest message
    status_log->ensureCursorVisible();
}
