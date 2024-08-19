/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "tabs/security_tab.h"
#include "tabs/ui_security_tab.h"

#include "adldap.h"
#include "select_dialogs/select_object_dialog.h"
#include "select_well_known_trustee_dialog.h"
#include "utils.h"
#include "permission_control_widgets/permissions_widget.h"

#include <QDebug>
#include <QLabel>
#include <QPersistentModelIndex>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>


SecurityTab::SecurityTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::SecurityTab();
    ui->setupUi(this);

    tab_edit = new SecurityTabEdit(this);

    edit_list->append({
        tab_edit,
    });

    sd = nullptr;

    trustee_model = new QStandardItemModel(0, 1, this);

    ui->trustee_view->setModel(trustee_model);

    permissions_widgets.append(ui->common_permissions_widget);
    permissions_widgets.append(ui->extended_permissions_widget);
    permissions_widgets.append(ui->creation_deletion_permissions_widget);
    permissions_widgets.append(ui->read_write_permissions_widget);
    permissions_widgets.append(ui->delegation_widget);
    for (PermissionsWidget *widget : permissions_widgets) {
        connect(widget, &PermissionsWidget::edited, tab_edit, &SecurityTabEdit::edited);
    }

    const QHash<QWidget*, PermissionsWidget*> tab_permission_wget_map = {
        {ui->common_permissions_tab, ui->common_permissions_widget},
        {ui->extended_permissions_tab, ui->extended_permissions_widget},
        {ui->creation_deletion_permissions_tab, ui->creation_deletion_permissions_widget},
        {ui->read_write_permissions_tab, ui->read_write_permissions_widget},
        {ui->delegation_tab, ui->delegation_widget}
    };

    connect(ui->permissions_tab_widget, &QTabWidget::currentChanged,
            [this, tab_permission_wget_map](int index) {
                QWidget *tab = ui->permissions_tab_widget->widget(index);
                if (!tab) {
                    return;
                }
                tab_permission_wget_map[tab]->update_permissions();
    });

    connect(
        ui->trustee_view->selectionModel(), &QItemSelectionModel::currentChanged,
        [this](){
            const QByteArray current_trustee = get_current_trustee();
            if (current_trustee.isEmpty()) {
                return;
            }
            for (PermissionsWidget *widget : permissions_widgets) {
                widget->set_current_trustee(current_trustee);
            }
    });

    connect(
        ui->add_trustee_button, &QAbstractButton::clicked,
        this, &SecurityTab::on_add_trustee_button);
    connect(
        ui->add_well_known_trustee_button, &QAbstractButton::clicked,
        this, &SecurityTab::on_add_well_known_trustee);
    connect(
        ui->remove_trustee_button, &QAbstractButton::clicked,
        this, &SecurityTab::on_remove_trustee_button);
}

void SecurityTab::load(AdInterface &ad, const AdObject &object) {
    security_descriptor_free(sd);
    sd = object.get_security_descriptor();

    const QStringList target_class_list = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    for (PermissionsWidget *widget : permissions_widgets) {
        widget->init(target_class_list, sd);
    }

    load_current_sd(ad);

    is_policy = object.is_class(CLASS_GP_CONTAINER);
}

SecurityTabEdit::SecurityTabEdit(SecurityTab *security_tab_arg)
: AttributeEdit(security_tab_arg), security_tab(security_tab_arg) {

}

SecurityTabEdit::~SecurityTabEdit() {
}

void SecurityTab::fix_acl_order() {
    security_descriptor_sort_dacl(sd);

    emit tab_edit->edited();
}

void SecurityTab::set_read_only() {
    ui->add_trustee_button->setEnabled(false);
    ui->add_well_known_trustee_button->setEnabled(false);
    ui->remove_trustee_button->setEnabled(false);

    for (PermissionsWidget *widget : permissions_widgets) {
        widget->set_read_only();
    }
}

bool SecurityTab::verify_acl_order() const {
    const bool out = security_descriptor_verify_acl_order(sd);

    return out;
}

SecurityTab::~SecurityTab() {
    delete ui;
    if (sd != nullptr) {
        security_descriptor_free(sd);
    }
}

// Load sd data into trustee model and right models
// NOTE: Rights loading is not explicitly called
// here but it is called implicitly because
// setCurrentIndex() emits currentChanged() signal
// which calls set_current_trustee() on permission widgets
void SecurityTab::load_current_sd(AdInterface &ad) {
    // Save previous selected trustee before reloading
    // trustee model. This is for the case where we
    // need to restore selection later.
    const QByteArray previous_selected_trustee = [&]() {
        const QList<QModelIndex> selected_list = ui->trustee_view->selectionModel()->selectedRows();

        if (!selected_list.isEmpty()) {
            const QModelIndex selected = selected_list[0];
            const QByteArray out = selected.data(TrusteeItemRole_Sid).toByteArray();

            return out;
        } else {
            return QByteArray();
        }
    }();

    // Load trustee model
    trustee_model->removeRows(0, trustee_model->rowCount());
    const QList<QByteArray> trustee_list = security_descriptor_get_trustee_list(sd);
    add_trustees(trustee_list, ad);

    // Select a trustee
    //
    // Note that trustee view must always have a
    // selection so that rights view displays
    // something. We also restore
    const QModelIndex selected_trustee = [&]() {
        const QModelIndex first_index = trustee_model->index(0, 0);

        // Restore previously selected trustee
        const QList<QModelIndex> match_list = trustee_model->match(first_index, TrusteeItemRole_Sid, previous_selected_trustee, -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));

        if (!match_list.isEmpty()) {
            return match_list[0];
        } else {
            return first_index;
        }
    }();

    ui->trustee_view->selectionModel()->setCurrentIndex(selected_trustee, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
}

void SecurityTabEdit::load(AdInterface &ad, const AdObject &object) {
    security_tab->load(ad, object);
}

bool SecurityTabEdit::verify(AdInterface &ad, const QString &target) const {
    UNUSED_ARG(target);
    if (security_tab->is_policy) {
        // To apply security tab for policies we need user
        // to have admin rights to be able to sync perms of
        // GPT
        const bool have_sufficient_rights = ad.logged_in_as_domain_admin();

        return have_sufficient_rights;
    } else {
        return true;
    }
}

bool SecurityTabEdit::apply(AdInterface &ad, const QString &target) const {
    bool total_success = true;

    total_success = (total_success && ad_security_replace_security_descriptor(ad, target, security_tab->sd));

    if (security_tab->is_policy) {
        total_success = (total_success && ad.gpo_sync_perms(target));
    }

    return total_success;
}

void SecurityTab::on_add_trustee_button() {
    auto dialog = new SelectObjectDialog({CLASS_USER, CLASS_GROUP}, SelectObjectDialogMultiSelection_Yes, ui->trustee_view);
    dialog->setWindowTitle(tr("Add Trustee"));
    dialog->open();

    connect(
        dialog, &SelectObjectDialog::accepted,
        this,
        [this, dialog]() {
            AdInterface ad;
            if (ad_failed(ad, ui->trustee_view)) {
                return;
            }

            // Get sid's of selected objects
            const QList<QByteArray> sid_list = [&, dialog]() {
                QList<QByteArray> out;

                const QList<QString> selected_list = dialog->get_selected();
                for (const QString &dn : selected_list) {
                    const AdObject object = ad.search_object(dn, {ATTRIBUTE_OBJECT_SID});
                    const QByteArray sid = object.get_value(ATTRIBUTE_OBJECT_SID);

                    out.append(sid);
                }

                return out;
            }();

            add_trustees(sid_list, ad);
        });
}

void SecurityTab::on_remove_trustee_button() {
    AdInterface ad;
    if (ad_failed(ad, ui->remove_trustee_button)) {
        return;
    }

    const QList<QByteArray> removed_trustee_list = [&]() {
        QList<QByteArray> out;

        QItemSelectionModel *selection_model = ui->trustee_view->selectionModel();
        const QList<QPersistentModelIndex> selected_list = persistent_index_list(selection_model->selectedRows());

        for (const QPersistentModelIndex &index : selected_list) {
            const QByteArray sid = index.data(TrusteeItemRole_Sid).toByteArray();

            out.append(sid);
        }

        return out;
    }();

    // Remove from sd
    security_descriptor_remove_trustee(sd, removed_trustee_list);

    // Reload sd
    //
    // NOTE: we do this instead of removing selected
    // indexes because not all trustee's are guaranteed
    // to have been removed
    load_current_sd(ad);

    const bool removed_any = !removed_trustee_list.isEmpty();

    if (removed_any) {
        emit tab_edit->edited();
    }
}

void SecurityTab::add_trustees(const QList<QByteArray> &sid_list, AdInterface &ad) {
    const QList<QString> current_sid_string_list = [&]() {
        QList<QString> out;

        for (int row = 0; row < trustee_model->rowCount(); row++) {
            QStandardItem *item = trustee_model->item(row, 0);
            const QByteArray sid = item->data(TrusteeItemRole_Sid).toByteArray();
            const QString sid_string = object_sid_display_value(sid);

            out.append(sid_string);
        }

        return out;
    }();

    bool added_anything = false;
    bool failed_to_add_because_already_exists = false;

    for (const QByteArray &sid : sid_list) {
        const QString sid_string = object_sid_display_value(sid);
        const bool trustee_already_in_list = (current_sid_string_list.contains(sid_string));
        if (trustee_already_in_list) {
            failed_to_add_because_already_exists = true;

            continue;
        }

        auto item = new QStandardItem();
        const QString name = ad_security_get_trustee_name(ad, sid);
        item->setText(name);
        item->setData(sid, TrusteeItemRole_Sid);
        trustee_model->appendRow(item);

        added_anything = true;
    }

    ui->trustee_view->sortByColumn(0, Qt::AscendingOrder);

    if (added_anything) {
        emit tab_edit->edited();
    }

    if (failed_to_add_because_already_exists) {
        message_box_warning(ui->trustee_view, tr("Error"), tr("Failed to add some trustee's because they are already in the list."));
    }
}

void SecurityTab::on_add_well_known_trustee() {
    auto dialog = new SelectWellKnownTrusteeDialog(ui->trustee_view);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog]() {
            AdInterface ad;
            if (ad_failed(ad, ui->trustee_view)) {
                return;
            }

            const QList<QByteArray> trustee_list = dialog->get_selected();

            add_trustees(trustee_list, ad);
        });
}

QByteArray SecurityTab::get_current_trustee() const {
    const QModelIndex current_index = ui->trustee_view->currentIndex();
    if (!current_index.isValid()) {
        return QByteArray();
    }

    QStandardItem *current_item = trustee_model->itemFromIndex(current_index);
    const QByteArray out = current_item->data(TrusteeItemRole_Sid).toByteArray();

    return out;
}
