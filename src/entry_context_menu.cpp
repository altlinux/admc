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

#include "entry_context_menu.h"
#include "ad_interface.h"
#include "settings.h"
#include "confirmation_dialog.h"
#include "move_dialog.h"
#include "utils.h"
#include "password_dialog.h"

#include <QString>
#include <QMessageBox>
#include <QInputDialog>
#include <QProcess>
#include <QDir>
#include <QPoint>
#include <QModelIndex>
#include <QAbstractItemView>

EntryContextMenu::EntryContextMenu(QWidget *parent)
: QMenu(parent)
{
    // NOTE: use parent, not context menu itself so dialog is centered
    move_dialog = new MoveDialog(parent);
}

// Open this context menu when view requests one
void EntryContextMenu::connect_view(QAbstractItemView *view, int dn_column) {
    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        [=]
        (const QPoint pos) {
            const QModelIndex base_index = view->indexAt(pos);

            if (!base_index.isValid()) {
                return;
            }

            const QPoint global_pos = view->mapToGlobal(pos);

            const QModelIndex index = convert_to_source(base_index);
            const QString dn = get_dn_from_index(index, dn_column);

            const QModelIndex parent_index = index.parent();
            QString parent_dn = "";
            if (parent_index.isValid()) {
                parent_dn = get_dn_from_index(parent_index, dn_column);
            }

            this->open(global_pos, dn, parent_dn);
        });
}

void EntryContextMenu::open(const QPoint &global_pos, const QString &dn, const QString &parent_dn) {
    clear();

    QAction *action_to_show_menu_at = addAction("Details", [this, dn]() {
        emit details(dn);
    });

    addAction("Delete", [this, dn]() {
        delete_entry(dn);
    });
    addAction("Rename", [this, dn]() {
        rename(dn);
    });

    QMenu *submenu_new = addMenu("New");
    submenu_new->addAction("New User", [this, dn]() {
        new_user(dn);
    });
    submenu_new->addAction("New Computer", [this, dn]() {
        new_computer(dn);
    });
    submenu_new->addAction("New Group", [this, dn]() {
        new_group(dn);
    });
    submenu_new->addAction("New OU", [this, dn]() {
        new_ou(dn);
    });

    addAction("Move", [this, dn]() {
        move_dialog->open_for_entry(dn, MoveDialogType_Move);
    });

    const bool is_policy = AD()->is_policy(dn); 
    const bool is_user = AD()->is_user(dn); 
    
    if (is_policy) {
        submenu_new->addAction("Edit Policy", [this, dn]() {
            edit_policy(dn);
        });
    }

    if (is_user) {
        addAction("Add to group", [this, dn]() {
            move_dialog->open_for_entry(dn, MoveDialogType_AddToGroup);
        });

        addAction("Reset password", [this, dn]() {
            const auto password_dialog = new PasswordDialog(dn, this);
            password_dialog->open();
        });
    }

    // Special contextual action
    // shown if parent is group and entry is user
    if (parent_dn != "") {
        const bool parent_is_group = AD()->is_group(parent_dn);

        if (is_user && parent_is_group) {
            addAction("Remove from group", [this, dn, parent_dn]() {
                AD()->group_remove_user(parent_dn, dn);
            });
        }
    }

    exec(global_pos, action_to_show_menu_at);
}

void EntryContextMenu::delete_entry(const QString &dn) {
    const QString name = AD()->attribute_get(dn, "name");
    const QString text = QString("Are you sure you want to delete \"%1\"?").arg(name);
    const bool confirmed = confirmation_dialog(text, this);

    if (confirmed) {
        AD()->object_delete(dn);
    }    
}

void EntryContextMenu::new_entry_dialog(const QString &parent_dn, NewEntryType type) {
    QString type_string = new_entry_type_to_string[type];
    QString dialog_title = "New " + type_string;
    QString input_label = type_string + " name";

    bool ok;
    QString name = QInputDialog::getText(this, dialog_title, input_label, QLineEdit::Normal, "", &ok);

    // TODO: maybe expand tree to newly created entry?

    // Create user once dialog is complete
    if (ok && !name.isEmpty()) {
        // Attempt to create user in AD

        const QMap<NewEntryType, QString> new_entry_type_to_suffix = {
            {NewEntryType::User, "CN"},
            {NewEntryType::Computer, "CN"},
            {NewEntryType::OU, "OU"},
            {NewEntryType::Group, "CN"},
        };
        QString suffix = new_entry_type_to_suffix[type];

        const QString dn = suffix + "=" + name + "," + parent_dn;

        AD()->object_create(name, dn, type);
    }
}

void EntryContextMenu::new_user(const QString &dn) {
    new_entry_dialog(dn, NewEntryType::User);
}

void EntryContextMenu::new_computer(const QString &dn) {
    new_entry_dialog(dn, NewEntryType::Computer);
}

void EntryContextMenu::new_group(const QString &dn) {
    new_entry_dialog(dn, NewEntryType::Group);
}

void EntryContextMenu::new_ou(const QString &dn) {
    new_entry_dialog(dn, NewEntryType::OU);
}

void EntryContextMenu::rename(const QString &dn) {
    // Get new name from input box
    QString dialog_title = "Rename";
    QString input_label = "New name:";
    bool ok;
    QString new_name = QInputDialog::getText(this, dialog_title, input_label, QLineEdit::Normal, "", &ok);

    if (ok && !new_name.isEmpty()) {
        AD()->object_rename(dn, new_name);
    }
}

void EntryContextMenu::edit_policy(const QString &dn) {
    // Start policy edit process
    const auto process = new QProcess(this);

    const QString program_name = "../gpgui";
    process->setProgram(QDir::currentPath() + program_name);

    const char *uri = "ldap://dc0.domain.alt";

    const QString path = AD()->attribute_get(dn, "gPCFileSysPath");

    QStringList args;
    args << uri;
    args << path;
    process->setArguments(args);

    printf("on_action_edit_policy\ndn=%s\npath=%s\n", qPrintable(dn), qPrintable(path));
    printf("execute command: %s %s %s\n", qPrintable(program_name), qPrintable(uri), qPrintable(path));

    process->start();
}
