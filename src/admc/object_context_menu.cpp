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
#include "select_dialog.h"
#include "rename_dialog.h"
#include "utils.h"
#include "password_dialog.h"
#include "create_dialog.h"
#include "settings.h"
#include "details_widget.h"
#include "ad_config.h"

#include <QString>
#include <QMessageBox>
#include <QInputDialog>
#include <QProcess>
#include <QDir>
#include <QPoint>
#include <QModelIndex>
#include <QAbstractItemView>

// Open this context menu when view requests one
void ObjectContextMenu::connect_view(QAbstractItemView *view, int dn_column) {
    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        [=]
        (const QPoint pos) {
            const QString dn = get_dn_from_pos(pos, view, dn_column);

            ObjectContextMenu context_menu(dn);
            exec_menu_from_view(&context_menu, view, pos);
        });
}

ObjectContextMenu::ObjectContextMenu(const QString &dn)
: QMenu()
{
    const AdObject object = AD()->request_all(dn);

    addAction(tr("Details"), [this, dn]() {
        DetailsWidget::open_for_target(dn);
    });

    if (object.is_policy()) {
        // TODO: policy version seems to be too disconnected from general object context menu, maybe just move it to policies widget?
        addAction(tr("Edit Policy"), [this, dn, object]() {
            edit_policy(dn, object);
        });
        addAction(tr("Rename"), [this, dn]() {
            auto rename_dialog = new RenameDialog(dn);
            rename_dialog->open();
        });
    } else {
        const bool cannot_move = object.get_system_flag(SystemFlagsBit_CannotMove);
        const bool cannot_rename = object.get_system_flag(SystemFlagsBit_CannotRename);
        const bool cannot_delete = object.get_system_flag(SystemFlagsBit_CannotDelete);

        auto delete_action = addAction(tr("Delete"), [this, dn, object]() {
            delete_object(dn, object);
        });
        if (cannot_delete) {
            delete_action->setEnabled(false);
        }

        auto rename_action = addAction(tr("Rename"), [this, dn]() {
            auto rename_dialog = new RenameDialog(dn);
            rename_dialog->open();
        });
        if (cannot_rename) {
            rename_action->setEnabled(false);
        }

        QMenu *submenu_new = addMenu("New");
        for (int i = 0; i < CreateType_COUNT; i++) {
            const CreateType type = (CreateType) i;
            const QString object_string = create_type_to_string(type);

            submenu_new->addAction(object_string, [dn, type]() {
                const auto create_dialog = new CreateDialog(dn, type);
                create_dialog->open();
            });
        }

        auto move_action = addAction(tr("Move"));
        connect(
            move_action, &QAction::triggered,
            [this, dn, object]() {
                move(dn, object);
            });
        if (cannot_move) {
            move_action->setEnabled(false);
        }

        if (object.is_user()) {
            QAction *add_to_group_action = addAction(tr("Add to group"));
            connect(
                add_to_group_action, &QAction::triggered,
                [this, dn]() {
                    add_to_group(dn);
                });

            addAction(tr("Reset password"), [dn]() {
                const auto password_dialog = new PasswordDialog(dn);
                password_dialog->open();
            });

            const bool disabled = object.get_account_option(AccountOption_Disabled);
            QString disable_text;
            if (disabled) {
                disable_text = tr("Enable account");
            } else {
                disable_text = tr("Disable account");
            }
            addAction(disable_text, [this, dn, disabled]() {
                AD()->user_set_account_option(dn, AccountOption_Disabled, !disabled);
            });
        }
    }
}

void ObjectContextMenu::delete_object(const QString &dn, const AdObject &object) {
    const QString name = object.get_string(ATTRIBUTE_NAME);
    const QString text = QString(tr("Are you sure you want to delete \"%1\"?")).arg(name);
    const bool confirmed = confirmation_dialog(text, this);

    if (confirmed) {
        AD()->object_delete(dn);
    }    
}

void ObjectContextMenu::edit_policy(const QString &dn, const AdObject &object) {
    // Start policy edit process
    const auto process = new QProcess();

    const QString path =
    [dn, object]() {
        QString path_tmp = object.get_string("gPCFileSysPath");
        path_tmp.replace("\\", "/");

        // TODO: file sys path as it is, is like this:
        // "smb://domain.alt/sysvol/domain.alt/Policies/{D7E75BC7-138D-4EE1-8974-105E4A2DE560}"
        // But that fails to load the whole directory sometimes
        // Replacing domain at the start with current host fixes it
        // "smb://dc0.domain.alt/sysvol/domain.alt/Policies/{D7E75BC7-138D-4EE1-8974-105E4A2DE560}"
        // not sure if this is required and which host/DC is the correct one
        const QString host = AD()->host();

        const int sysvol_i = path_tmp.indexOf("sysvol");
        path_tmp.remove(0, sysvol_i);

        path_tmp = QString("smb://%1/%2").arg(host, path_tmp);

        return path_tmp;
    }();

    const QString program_name = "/home/kevl/admc/build/gpgui";

    QStringList args = {"-p", path};

    qint64 pid;
    const bool start_success = process->startDetached(program_name, args, QString(), &pid);

    printf("edit_policy\n");
    printf("path=%s\n", qPrintable(path));
    printf("pid=%lld\n", pid);
    printf("start_success=%d\n", start_success);
}

void ObjectContextMenu::move(const QString &dn, const AdObject &object) {
    const QString object_category = object.get_string(ATTRIBUTE_OBJECT_CATEGORY);
    const QList<QString> possible_superiors = ADCONFIG()->get_possible_superiors(object_category);

    const QList<QString> selected_objects = SelectDialog::open(possible_superiors);

    if (selected_objects.size() == 1) {
        const QString container = selected_objects[0];

        AD()->object_move(dn, container);
    }
}

void ObjectContextMenu::add_to_group(const QString &dn) {
    const QList<QString> classes = {CLASS_GROUP};
    const QList<QString> selected_objects = SelectDialog::open(classes, SelectDialogMultiSelection_Yes);

    if (selected_objects.size() > 0) {
        for (auto group : selected_objects) {
            AD()->group_add_user(group, dn);
        }
    }
}
