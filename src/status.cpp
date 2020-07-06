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

#include "status.h"
#include "ad_interface.h"
#include "settings.h"

#include <QStatusBar>
#include <QTextEdit>
#include <QAction>

Status::Status(QStatusBar *status_bar_arg, QTextEdit *status_log_arg, QObject *parent)
: QObject(parent)
{
    status_bar = status_bar_arg;
    status_log = status_log_arg;

    message(tr("Ready"), 10 * 1000);

    connect(
        SETTINGS()->toggle_show_status_log, &QAction::toggled,
        this, &Status::on_toggle_show_status_log);

    connect(
        AD(), &AdInterface::ad_interface_login_complete,
        [this] (const QString &search_base, const QString &head_dn) {
            message(QString("Logged in to \"%1\" with head dn at \"%2\"").arg(search_base, head_dn));
        });
    connect(
        AD(), &AdInterface::ad_interface_login_failed,
        [this] (const QString &base, const QString &head) {
            message(QString("Failed to login to \"%1\" with head dn at \"%2\"").arg(base, head));
        });

    connect(
        AD(), &AdInterface::load_children_failed,
        [this] (const QString &dn, const QString &error_str) {
            message(QString("Failed to load children of \"%1\". Error: \"%2\"").arg(dn, error_str));
        });

    connect(
        AD(), &AdInterface::search_failed,
        [this] (const QString &filter, const QString &error_str) {
            message(QString("Failed to search for \"%1\". Error: \"%2\"").arg(filter, error_str));
        });

    connect(
        AD(), &AdInterface::load_attributes_failed,
        [this] (const QString &dn, const QString &error_str) {
            message(QString("Failed to load attributes of \"%1\". Error: \"%2\"").arg(dn, error_str));
        });

    connect(
        AD(), &AdInterface::delete_entry_complete,
        [this] (const QString &dn) {
            message(QString("Deleted entry \"%1\"").arg(dn));
        });
    connect(
        AD(), &AdInterface::delete_entry_failed,
        [this] (const QString &dn, const QString &error_str) {
            message(QString("Failed to delete entry \"%1\". Error: \"%2\"").arg(dn, error_str));
        });

    connect(
        AD(), &AdInterface::set_attribute_complete,
        [this] (const QString &dn, const QString &attribute, const QString &old_value, const QString &value) {
            message(QString("Changed attribute \"%1\" of \"%2\" from \"%3\" to \"%4\"").arg(attribute, dn, old_value, value));
        });
    connect(
        AD(), &AdInterface::set_attribute_failed,
        [this] (const QString &dn, const QString &attribute, const QString &old_value, const QString &value, const QString &error_str) {
            message(QString("Failed to change attribute \"%1\" of entry \"%2\" from \"%3\" to \"%4\". Error: \"%5\"").arg(attribute, dn, old_value, value, error_str));
        });

    connect(
        AD(), &AdInterface::create_entry_complete,
        [this] (const QString &dn, NewEntryType type) {
            QString type_str = new_entry_type_to_string[type];
            message(QString("Created entry \"%1\" of type \"%2\"").arg(dn, type_str));
        });
    connect(
        AD(), &AdInterface::create_entry_failed,
        [this] (const QString &dn, NewEntryType type, const QString &error_str) {
            QString type_str = new_entry_type_to_string[type];
            message(QString("Failed to create entry \"%1\" of type \"%2\". Error: \"%3\"").arg(dn, type_str, error_str));
        });

    connect(
        AD(), &AdInterface::move_complete,
        [this] (const QString &dn, const QString &new_container, const QString &new_dn) {
            message(QString("Moved \"%1\" to \"%2\"").arg(dn).arg(new_container));
        });
    connect(
        AD(), &AdInterface::move_failed,
        [this] (const QString &dn, const QString &new_container, const QString &new_dn, const QString &error_str) {
            message(QString("Failed to move \"%1\" to \"%2\". Error: \"%3\"").arg(dn, new_container, error_str));
        });

    connect(
        AD(), &AdInterface::add_user_to_group_complete,
        [this] (const QString &group_dn, const QString &user_dn) {
            message(QString("Added user \"%1\" to group \"%2\"").arg(user_dn, group_dn));
        });
    connect(
        AD(), &AdInterface::add_user_to_group_failed,
        [this] (const QString &group_dn, const QString &user_dn, const QString &error_str) {
            message(QString("Failed to add user \"%1\" to group \"%2\". Error: \"%3\"").arg(user_dn, group_dn, error_str));
        });

    connect(
        AD(), &AdInterface::group_remove_user_complete,
        [this] (const QString &group_dn, const QString &user_dn) {
            message(QString("Removed user \"%1\" from group \"%2\"").arg(user_dn, group_dn));
        });
    connect(
        AD(), &AdInterface::group_remove_user_failed,
        [this] (const QString &group_dn, const QString &user_dn, const QString &error_str) {
            message(QString("Failed to remove user \"%1\" from group \"%2\". Error: \"%3\"").arg(user_dn, group_dn, error_str));
        });

    connect(
        AD(), &AdInterface::rename_complete,
        [this] (const QString &dn, const QString &new_name, const QString &new_dn) {
            message(QString("Renamed \"%1\" to \"%2\"").arg(dn, new_name));
        });
    connect(
        AD(), &AdInterface::rename_failed,
        [this] (const QString &dn, const QString &new_name, const QString &new_dn, const QString &error_str) {
            message(QString("Failed to rename \"%1\" to \"%2\". Error: \"%3\"").arg(dn, new_name, error_str));
        });

    connect(
        AD(), &AdInterface::set_pass_complete,
        [this] (const QString &dn, const QString &) {
            message(QString("Set pass of \"%1\"").arg(dn));
        });
    connect(
        AD(), &AdInterface::set_pass_failed,
        [this] (const QString &dn, const QString &, const QString &error_str) {
            message(QString("Failed to set pass of \"%1\". Error: \"%2\"").arg(dn, error_str));
        });
}

void Status::on_toggle_show_status_log(bool checked) {
    if (checked) {
        status_log->setVisible(true); 
    } else {
        status_log->setVisible(false); 
    }
}

void Status::message(const QString &msg, int status_bar_timeout) {
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
