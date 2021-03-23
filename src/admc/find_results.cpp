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

#include "find_results.h"
#include "object_menu.h"
#include "properties_dialog.h"
#include "utils.h"
#include "ad/ad_interface.h"
#include "ad/ad_filter.h"
#include "ad/ad_config.h"
#include "ad/ad_object.h"
#include "object_model.h"
#include "settings.h"
#include "console_widget/customize_columns_dialog.h"
#include "console_widget/results_view.h"
#include "status.h"
#include "rename_dialog.h"
#include "select_container_dialog.h"
#include "create_dialog.h"
#include "select_dialog.h"
#include "password_dialog.h"

#include <QTreeView>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>
#include <QVBoxLayout>
#include <QMenu>
#include <QStandardItemModel>
#include <QHash>

QList<QString> get_dns(const QList<QModelIndex> &indexes) {
    QList<QString> out;

    for (const QModelIndex index : indexes) {
        const QString dn = index.data(ObjectRole_DN).toString();

        out.append(dn);
    }

    return out;  
}

FindResults::FindResults()
: QWidget()
{   
    submenu_new = new QMenu(tr("&New"));
    static const QList<QString> create_classes = {
        CLASS_USER,
        CLASS_COMPUTER,
        CLASS_OU,
        CLASS_GROUP,
    };
    for (const auto &object_class : create_classes) {
        const QString action_text = ADCONFIG()->get_class_display_name(object_class);

        submenu_new->addAction(action_text,
            [=]() {
                create(object_class);
            });
    }

    properties_action = new QAction(tr("&Properties"));
    delete_action = new QAction(tr("&Delete"));
    rename_action = new QAction(tr("&Rename"));
    move_action = new QAction(tr("&Move"));
    add_to_group_action = new QAction(tr("&Add to group"));
    enable_action = new QAction(tr("&Enable account"));
    disable_action = new QAction(tr("D&isable account"));
    reset_password_action = new QAction(tr("Reset &Password"));

    model = new QStandardItemModel(this);

    const QList<QString> header_labels = object_model_header_labels();
    model->setHorizontalHeaderLabels(header_labels);

    view = new ResultsView(this);
    view->set_model(model);

    object_count_label = new QLabel();

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(object_count_label);
    layout->addWidget(view);

    customize_columns_action = new QAction(tr("&Customize columns"), this);

    connect(
        customize_columns_action, &QAction::triggered,
        this, &FindResults::customize_columns);
    connect(
        view, &ResultsView::context_menu,
        this, &FindResults::on_context_menu);
    connect(
        view, &ResultsView::activated,
        this, &FindResults::properties);

    connect(
        properties_action, &QAction::triggered,
        this, &FindResults::properties);
    connect(
        delete_action, &QAction::triggered,
        this, &FindResults::delete_objects);
    connect(
        rename_action, &QAction::triggered,
        this, &FindResults::rename);
    connect(
        move_action, &QAction::triggered,
        this, &FindResults::move);
    connect(
        add_to_group_action, &QAction::triggered,
        this, &FindResults::add_to_group);
    connect(
        enable_action, &QAction::triggered,
        this, &FindResults::enable);
    connect(
        disable_action, &QAction::triggered,
        this, &FindResults::disable);
    connect(
        reset_password_action, &QAction::triggered,
        this, &FindResults::reset_password);

    connect(
        view, &ResultsView::selection_changed,
        this, &FindResults::update_actions_visibility);
}

void FindResults::add_actions_to_action_menu(QMenu *menu) {
    // Container
    menu->addMenu(submenu_new);

    menu->addSeparator();

    // User
    menu->addAction(add_to_group_action);
    menu->addAction(enable_action);
    menu->addAction(disable_action);
    menu->addAction(reset_password_action);

    menu->addSeparator();

    // General object
    menu->addAction(delete_action);
    menu->addAction(rename_action);
    menu->addAction(move_action);
}

void FindResults::add_actions_to_view_menu(QMenu *menu) {
    menu->addAction(customize_columns_action);
}

void FindResults::clear() {
    object_count_label->clear();
    model->removeRows(0, model->rowCount());
}

void FindResults::load(const QHash<QString, AdObject> &search_results) {
    for (const AdObject &object : search_results) {
        const QList<QStandardItem *> row = make_item_row(ADCONFIG()->get_columns().count());

        load_object_row(row, object);

        model->appendRow(row);
    }

    model->sort(0, Qt::AscendingOrder);

    const QString label_text = tr("%n object(s)", "", model->rowCount());
    object_count_label->setText(label_text);
}

QList<QList<QStandardItem *>> FindResults::get_selected_rows() const {
    const QList<QModelIndex> selected_rows = view->current_view()->selectionModel()->selectedRows();

    QList<QList<QStandardItem *>> out;

    for (const QModelIndex row_index : selected_rows) {
        const int row = row_index.row();

        QList<QStandardItem *> row_copy;

        for (int col = 0; col < model->columnCount(); col++) {
            QStandardItem *item = model->item(row, col);
            QStandardItem *item_copy = item->clone();
            row_copy.append(item_copy);
        }

        out.append(row_copy);
    }

    return out;
}

void FindResults::delete_objects() {
    const QList<QModelIndex> indexes = view->current_view()->selectionModel()->selectedRows();

    const QString text = QString(tr("Are you sure you want to delete this object?"));
    const bool confirmed = confirmation_dialog(text, this);

    if (confirmed) {
        AdInterface ad;
        if (ad_failed(ad)) {
            return;
        }

        for (const QModelIndex &index : indexes) {
            const QString dn = index.data(ObjectRole_DN).toString();
            ad.object_delete(dn);
        }

        STATUS()->display_ad_messages(ad, this);
    }
}

void FindResults::properties() {
    const QList<QModelIndex> indexes = view->current_view()->selectionModel()->selectedRows();
    if (indexes.size() != 1) {
        return;
    }
    const QModelIndex index = indexes[0];

    const QString dn = index.data(ObjectRole_DN).toString();

    PropertiesDialog *dialog = PropertiesDialog::open_for_target(dn);
    dialog->open();

    update_actions_visibility();
}

void FindResults::rename() {
    const QList<QModelIndex> indexes = view->current_view()->selectionModel()->selectedRows();
    if (indexes.size() != 1) {
        return;
    }
    const QModelIndex index = indexes[0];

    const QString dn = index.data(ObjectRole_DN).toString();

    auto dialog = new RenameDialog(dn, this);
    dialog->open();
}

void FindResults::create(const QString &object_class) {
    const QList<QModelIndex> selected_indexes = view->current_view()->selectionModel()->selectedRows();
    if (selected_indexes.size() == 0) {
        return;
    }

    const QModelIndex selected_index = selected_indexes[0];
    const QString selected_dn = selected_index.data(ObjectRole_DN).toString();

    const auto dialog = new CreateDialog(selected_dn, object_class, this);

    dialog->open();
}

void FindResults::move() {
    const QList<QModelIndex> selected_indexes = view->current_view()->selectionModel()->selectedRows();
    if (selected_indexes.size() == 0) {
        return;
    }

    auto dialog = new SelectContainerDialog(this);

    connect(
        dialog, &SelectContainerDialog::accepted,
        [=]() {
            const QString selected_container_dn = dialog->get_selected();

            AdInterface ad;
            if (ad_connected(ad)) {
                for (const QModelIndex &index : selected_indexes) {
                    const QString dn = index.data(ObjectRole_DN).toString();

                    ad.object_move(dn, selected_container_dn);
                }

                STATUS()->display_ad_messages(ad, this);
            }
        });

    dialog->open();
}

void FindResults::add_to_group() {
    auto dialog = new SelectDialog({CLASS_GROUP}, SelectDialogMultiSelection_Yes, this);

    QObject::connect(
        dialog, &SelectDialog::accepted,
        [=]() {
            AdInterface ad;
            if (ad_failed(ad)) {
                return;
            }

            const QList<QString> groups = dialog->get_selected();
            const QList<QModelIndex> selected_indexes = view->current_view()->selectionModel()->selectedRows();
            const QList<QString> targets = get_dns(selected_indexes);

            for (const QString &target : targets) {
                for (auto group : groups) {
                    ad.group_add_member(group, target);
                }
            }

            STATUS()->display_ad_messages(ad, this);
        });

    dialog->open();
}

void FindResults::enable() {
    enable_disable_helper(false);
}

void FindResults::disable() {
    enable_disable_helper(true);
}

void FindResults::reset_password() {
    const QList<QModelIndex> selected_indexes = view->current_view()->selectionModel()->selectedRows();
    const QList<QString> targets = get_dns(selected_indexes);

    if (targets.size() != 1) {
        return;
    }

    const QString target = targets[0];

    const auto password_dialog = new PasswordDialog(target, this);
    password_dialog->open();
}

void FindResults::customize_columns() {
    auto dialog = new CustomizeColumnsDialog(view->detail_view(), object_model_default_columns(), this);
    dialog->open();
}

void FindResults::on_context_menu(const QPoint pos) {
    const QPoint global_pos = view->mapToGlobal(pos);

    emit context_menu(global_pos);
}

void FindResults::enable_disable_helper(const bool disabled) {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QList<QModelIndex> selected_indexes = view->current_view()->selectionModel()->selectedRows();

    for (const QModelIndex &index : selected_indexes) {
        const QString dn = index.data(ObjectRole_DN).toString();
        const bool success = ad.user_set_account_option(dn, AccountOption_Disabled, disabled);

        if (success) {
            model->setData(index, disabled, ObjectRole_AccountDisabled);
        }
    }

    STATUS()->display_ad_messages(ad, this);

    update_actions_visibility();
}

// First, hide all actions, then show whichever actions are
// appropriate for current console selection
void FindResults::update_actions_visibility() {
    const QList<QAction *> all_actions ={
        submenu_new->menuAction(),
        delete_action,
        rename_action,
        move_action,
        add_to_group_action,
        enable_action,
        disable_action,
        reset_password_action,
    };
    for (QAction *action : all_actions) {
        action->setVisible(false);
    }

    const QList<QModelIndex> selected_list = view->current_view()->selectionModel()->selectedRows();

    // Get info about selected objects from view
    const QList<QString> targets =
    [=]() {
        QList<QString> out;

        for (const QModelIndex index : selected_list) {
            const QString dn = index.data(ObjectRole_DN).toString();

            out.append(dn);
        }

        return out;  
    }();

    const QSet<QString> target_classes =
    [=]() {
        QSet<QString> out;
        
        for (const QModelIndex index : selected_list) {
            const QList<QString> object_classes = index.data(ObjectRole_ObjectClasses).toStringList();
            const QString main_object_class = object_classes.last();

            out.insert(main_object_class);
        }

        return out;
    }();

    const bool single_object = (targets.size() == 1);

    if (single_object) {
        const QModelIndex index = selected_list[0];
        const QString target = targets[0];
        const QString target_class = target_classes.values()[0];

        // Get info about object that will determine which
        // actions are present/enabled
        const bool is_container =
        [=]() {
            const QList<QString> container_classes = ADCONFIG()->get_filter_containers();

            return container_classes.contains(target_class);
        }();

        const bool is_user = (target_class == CLASS_USER);

        const bool cannot_move = index.data(ObjectRole_CannotMove).toBool();
        const bool cannot_rename = index.data(ObjectRole_CannotRename).toBool();
        const bool cannot_delete = index.data(ObjectRole_CannotDelete).toBool();
        const bool account_disabled = index.data(ObjectRole_AccountDisabled).toBool();

        if (is_container) {
            submenu_new->menuAction()->setVisible(true);
        }

        if (is_user) {
            add_to_group_action->setVisible(true);
            reset_password_action->setVisible(true);

            if (account_disabled) {
                enable_action->setVisible(true);
            } else {
                disable_action->setVisible(true);
            }
        }

        move_action->setVisible(true);
        delete_action->setVisible(true);
        rename_action->setVisible(true);

        move_action->setEnabled(cannot_move);
        delete_action->setEnabled(cannot_delete);
        rename_action->setEnabled(cannot_rename);
    } else if (targets.size() > 1) {
        const bool all_users = (target_classes.contains(CLASS_USER) && target_classes.size() == 1);

        if (all_users) {
            add_to_group_action->setVisible(true);

            // NOTE: show both enable/disable for multiple
            // users because some users might be disabled,
            // some enabled and we want to provide a way to
            // disable all or enable all
            enable_action->setVisible(true);
            disable_action->setVisible(true);
        }

        move_action->setVisible(true);
        delete_action->setVisible(true);
    }
}
