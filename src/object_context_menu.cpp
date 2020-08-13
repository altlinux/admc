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

#include "object_context_menu.h"
#include "ad_interface.h"
#include "confirmation_dialog.h"
#include "move_dialog.h"
#include "rename_dialog.h"
#include "utils.h"
#include "password_dialog.h"
#include "settings.h"

#include <QString>
#include <QMessageBox>
#include <QInputDialog>
#include <QProcess>
#include <QDir>
#include <QPoint>
#include <QModelIndex>
#include <QAbstractItemView>

QString get_new_object_display_string(NewObjectType type);
void force_reload_attributes_and_diff(const QString &dn);

ObjectContextMenu::ObjectContextMenu(QWidget *parent)
: QMenu(parent)
{
    // NOTE: use parent, not context menu itself so dialog is centered
    move_dialog = new MoveDialog(parent);
}

// Open this context menu when view requests one
void ObjectContextMenu::connect_view(QAbstractItemView *view, int dn_column) {
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

void ObjectContextMenu::open(const QPoint &global_pos, const QString &dn, const QString &parent_dn) {
    clear();

    QAction *action_to_show_menu_at = addAction(tr("Details"), [this, dn]() {
        emit details(dn);
    });

    addAction(tr("Delete"), [this, dn]() {
        delete_object(dn);
    });
    addAction(tr("Rename"), [this, dn]() {
        auto rename_dialog = new RenameDialog(dn, this);
        rename_dialog->open();
    });

    QMenu *submenu_new = addMenu("New");
    for (int i = 0; i < NewObjectType::COUNT; i++) {
        const NewObjectType type = (NewObjectType) i;
        const QString object_string = get_new_object_display_string(type);

        submenu_new->addAction(object_string, [this, dn, type]() {
            new_object_dialog(dn, type);
        });
    }

    addAction(tr("Move"), [this, dn]() {
        move_dialog->open_for_object(dn, MoveDialogType_Move);
    });

    const bool is_policy = AdInterface::instance()->is_policy(dn); 
    const bool is_user = AdInterface::instance()->is_user(dn); 
    
    if (is_policy) {
        submenu_new->addAction(tr("Edit Policy"), [this, dn]() {
            edit_policy(dn);
        });
    }

    if (is_user) {
        addAction(tr("Add to group"), [this, dn]() {
            move_dialog->open_for_object(dn, MoveDialogType_AddToGroup);
        });

        addAction(tr("Reset password"), [this, dn]() {
            const auto password_dialog = new PasswordDialog(dn, this);
            password_dialog->open();
        });

        const bool disabled = AdInterface::instance()->user_get_account_option(dn, AccountOption_Disabled);
        QString disable_text;
        if (disabled) {
            disable_text = tr("Enable account");
        } else {
            disable_text = tr("Disable account");
        }
        addAction(disable_text, [this, dn, disabled]() {
            AdInterface::instance()->user_set_account_option(dn, AccountOption_Disabled, !disabled);
        });
    }

    // Special contextual action
    // shown if parent is group and object is user
    if (parent_dn != "") {
        const bool parent_is_group = AdInterface::instance()->is_group(parent_dn);

        if (is_user && parent_is_group) {
            addAction(tr("Remove from group"), [this, dn, parent_dn]() {
                AdInterface::instance()->group_remove_user(parent_dn, dn);
            });
        }
    }

    const bool dev_mode = Settings::instance()->get_bool(BoolSetting_DevMode);
    if (dev_mode) {
        addAction(tr("Force reload attributes and diff"), [dn]() {
            force_reload_attributes_and_diff(dn);
        });
    }

    exec(global_pos, action_to_show_menu_at);
}

void ObjectContextMenu::delete_object(const QString &dn) {
    const QString name = AdInterface::instance()->attribute_get(dn, ATTRIBUTE_NAME);
    const QString text = QString(tr("Are you sure you want to delete \"%1\"?")).arg(name);
    const bool confirmed = confirmation_dialog(text, this);

    if (confirmed) {
        AdInterface::instance()->object_delete(dn);
    }    
}

void ObjectContextMenu::new_object_dialog(const QString &parent_dn, NewObjectType type) {
    const QString dialog_title = tr("New object: ") + get_new_object_display_string(type);
    const QString input_label = tr("Name");

    bool ok;
    QString name = QInputDialog::getText(this, dialog_title, input_label, QLineEdit::Normal, "", &ok);

    // TODO: maybe expand tree to newly created object?

    if (ok && !name.isEmpty()) {
        const QMap<NewObjectType, QString> new_object_type_to_suffix = {
            {NewObjectType::User, "CN"},
            {NewObjectType::Computer, "CN"},
            {NewObjectType::OU, "OU"},
            {NewObjectType::Group, "CN"},
        };
        QString suffix = new_object_type_to_suffix[type];

        const QString dn = suffix + "=" + name + "," + parent_dn;

        AdInterface::instance()->object_create(name, dn, type);
    }
}

void ObjectContextMenu::edit_policy(const QString &dn) {
    // Start policy edit process
    const auto process = new QProcess(this);

    const QString program_name = "../gpgui";
    process->setProgram(QDir::currentPath() + program_name);

    const char *uri = "ldap://dc0.domain.alt";

    const QString path = AdInterface::instance()->attribute_get(dn, "gPCFileSysPath");

    QStringList args;
    args << uri;
    args << path;
    process->setArguments(args);

    printf("on_action_edit_policy\ndn=%s\npath=%s\n", qPrintable(dn), qPrintable(path));
    printf("execute command: %s %s %s\n", qPrintable(program_name), qPrintable(uri), qPrintable(path));

    process->start();
}

QString get_new_object_display_string(NewObjectType type) {
    switch (type) {
        case NewObjectType::User: return ObjectContextMenu::tr("User");
        case NewObjectType::Computer: return ObjectContextMenu::tr("Computer");
        case NewObjectType::OU: return ObjectContextMenu::tr("Organization Unit");
        case NewObjectType::Group: return ObjectContextMenu::tr("Group");
        case NewObjectType::COUNT: return "COUNT";
    }
    return "";
}

void force_reload_attributes_and_diff(const QString &dn) {

    QMap<QString, QList<QString>> old_attributes = AdInterface::instance()->get_all_attributes(dn);

    AdInterface::instance()->update_cache({dn});

    QList<QString> changes;

    QMap<QString, QList<QString>> new_attributes = AdInterface::instance()->get_all_attributes(dn);

    for (auto &attribute : old_attributes.keys()) {
        const QString old_value = old_attributes[attribute][0];

        if (new_attributes.contains(attribute)) {
            const QString new_value = new_attributes[attribute][0];
            
            if (new_value != old_value) {
                changes.append(QString("\"%1\": \"%2\"->\"%3\"").arg(attribute, old_value, new_value));
            }
        } else {
            changes.append(QString("\"%1\": \"%2\"->\"unset\"").arg(attribute, old_value));
        }
    }

    for (auto &attribute : old_attributes.keys()) {
        if (!old_attributes.contains(attribute)) {
            const QString new_value = new_attributes[attribute][0];
            changes.append(QString("\"%1\": \"unset\"->\"%2\"").arg(attribute, new_value));
        }
    }

    if (!changes.isEmpty()) {
        const QString name = AdInterface::instance()->attribute_get(dn, ATTRIBUTE_NAME);

        printf("Attributes of object \"%s\" changed:\n", qPrintable(name));

        for (auto change : changes) {
            printf("%s\n", qPrintable(change));
        }
    }
}
