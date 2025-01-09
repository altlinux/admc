/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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
#include "globals.h"
#include "permission_control_widgets/sddl_view_dialog.h"

#include <QDebug>
#include <QLabel>
#include <QPersistentModelIndex>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <algorithm>
#include <QMenu>


SecurityTab::SecurityTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent), sd(nullptr), previous_sd(nullptr) {
    ui = new Ui::SecurityTab();
    ui->setupUi(this);

    tab_edit = new SecurityTabEdit(this);

    edit_list->append({
        tab_edit,
    });

    trustee_model = new QStandardItemModel(0, 1, this);

    ui->trustee_view->setModel(trustee_model);

    sddl_view = new SDDLViewDialog(this);

    permissions_widgets.append(ui->common_permissions_widget);
    permissions_widgets.append(ui->extended_permissions_widget);
    permissions_widgets.append(ui->creation_deletion_permissions_widget);
    permissions_widgets.append(ui->read_write_permissions_widget);
    permissions_widgets.append(ui->delegation_widget);
    for (PermissionsWidget *widget : permissions_widgets) {
        connect(widget, &PermissionsWidget::edited,
                [this, widget](){
                    tab_edit->edited();
                    if ((!ui->clear_all_button->isEnabled() && widget->there_are_selected_permissions())) {
                        ui->clear_all_button->setEnabled(true);
                    }
                    sddl_view->update(sd);
        });
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
            bool clear_enabled = false;
            for (PermissionsWidget *widget : permissions_widgets) {
                widget->set_current_trustee(current_trustee);

                if (widget->there_are_selected_permissions()) {
                    clear_enabled = true;
                }
            }
            ui->clear_all_button->setEnabled(clear_enabled);

            sddl_view->set_trustee(current_trustee);
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

    connect(
        ui->applied_objects_cmbBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, &SecurityTab::on_applied_objs_cmbbox);
    connect(
        ui->clear_all_button, &QAbstractButton::clicked,
        this, &SecurityTab::on_clear_all);

    QMenu *more_menu = new QMenu(this);
    QAction *show_sddl_action = new QAction(tr("Show descriptor in SDDL"));
    restore_sd_action = new QAction(tr("Rollback to the previous descriptor"));
    more_menu->addActions({show_sddl_action, restore_sd_action});
    connect(show_sddl_action, &QAction::triggered, this, &SecurityTab::on_show_sddl_sd);
    connect(restore_sd_action, &QAction::triggered, this, &SecurityTab::on_restore_previous_sd);

    ui->more_button->setMenu(more_menu);
    connect(more_menu, &QMenu::aboutToShow, this, &SecurityTab::on_more_menu);
}

void SecurityTab::load(AdInterface &ad, const AdObject &object) {
    // TODO: Remove security tab self reload after changes applying (because its excessive).
    // Probably this should be done for other tabs too.

    security_descriptor_free(sd);
    sd = object.get_security_descriptor();

    sddl_view->update(sd);

    target_class_list = object.get_strings(ATTRIBUTE_OBJECT_CLASS);

    for (PermissionsWidget *widget : permissions_widgets) {
        widget->init(target_class_list, sd);
    }

    load_sd(ad, sd);

    if (ui->applied_objects_cmbBox->count() == 0) {
        load_applied_objects_cmbbox(target_class_list);
    }
    else {
        // Update rights model with current combo box value
        on_applied_objs_cmbbox();
    }

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
    security_descriptor_free(sd);
    security_descriptor_free(previous_sd);
}

// Load sd data into trustee model and right models
// NOTE: Rights loading is not explicitly called
// here but it is called implicitly because
// setCurrentIndex() emits currentChanged() signal
// which calls set_current_trustee() on permission widgets
void SecurityTab::load_sd(AdInterface &ad, security_descriptor *sd_arg) {
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
    const QList<QByteArray> trustee_list = security_descriptor_get_trustee_list(sd_arg);
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

void SecurityTab::load_applied_objects_cmbbox(const QStringList &target_class_list) {
    ui->applied_objects_cmbBox->addItem(tr("This object"), (int)AppliedObjects_ThisObject);
    ui->applied_objects_cmbBox->addItem(tr("This object and all child objects"), (int)AppliedObjects_ThisAndChildObjects);
    ui->applied_objects_cmbBox->addItem(tr("All child objects"), (int)AppliedObjects_AllChildObjects);

    const QString target_class = target_class_list.last();
    QStringList inferior_class_list = g_adconfig->all_inferiors_list(target_class);

    QHash<QString, QString> text_class_map;
    QStringList translated_class_names;
    QStringList not_translated_class_names;
    for (auto inferior : inferior_class_list) {
        // Dont add auxiliary classes that have no assignable permissions
        if (g_adconfig->class_is_auxiliary(inferior)) {
            continue;
        }

        const QString text = g_adconfig->get_class_display_name(inferior);
        text_class_map[text] = inferior;
        if (text != inferior) {
            translated_class_names.append(text);
        }
        else {
            not_translated_class_names.append(text);
        }
    }
    // Sort translated and not translated classes separately to put first on second
    std::sort(translated_class_names.begin(), translated_class_names.end(), std::less<QString>());
    std::sort(not_translated_class_names.begin(), not_translated_class_names.end(), std::less<QString>());

    QStringList sorted_class_names = translated_class_names + not_translated_class_names;

    for (const QString &class_name : sorted_class_names) {
        const QString item_text = tr("Child objects: ") + class_name;
        ui->applied_objects_cmbBox->addItem(item_text, (int)AppliedObjects_ChildObjectClass);
        ui->applied_objects_cmbBox->setItemData(ui->applied_objects_cmbBox->count() - 1,
                                                text_class_map[class_name], AppliedObjectRole_ObjectClass);
    }
}

void SecurityTab::on_applied_objs_cmbbox() {
    AppliedObjects applied_objs = (AppliedObjects)ui->applied_objects_cmbBox->currentData().toInt();
    int task_delegation_tab_index = ui->permissions_tab_widget->indexOf(ui->delegation_tab);
    if (applied_objs != AppliedObjects_ThisObject) {
        // NOTE: Task delegation tab is hidden because common tasks are set of predefined rights
        // and should not vary depending on applied object(s) value
        ui->permissions_tab_widget->setTabEnabled(task_delegation_tab_index, false);
    }
    else {
        ui->permissions_tab_widget->setTabEnabled(task_delegation_tab_index, true);
    }

    const QString obj_class = ui->applied_objects_cmbBox->currentData(AppliedObjectRole_ObjectClass).
            toString();
    for (PermissionsWidget* perm_wget : permissions_widgets) {
       perm_wget->update_permissions(applied_objs, obj_class);
    }
}

void SecurityTab::on_clear_all() {
    const QByteArray trustee = get_current_trustee();
    if (trustee.isEmpty()) {
        return;
    }

    // Do not reload sd to leave permission selection enabled for
    // trustee removed from security descriptor.
    security_descriptor_remove_trustee(sd, {trustee});
    for (PermissionsWidget* perm_wget : permissions_widgets) {
       perm_wget->update_permissions();
    }

    tab_edit->edited();
    ui->clear_all_button->setDisabled(true);
}

void SecurityTab::on_show_sddl_sd() {
    sddl_view->show();
}

void SecurityTab::on_restore_previous_sd() {
    if (previous_sd == nullptr) {
        return;
    }

    show_busy_indicator();

    AdInterface ad;
    if (!ad.is_connected()) {
        hide_busy_indicator();
        return;
    }

    security_descriptor_free(sd);
    sd = security_descriptor_copy(previous_sd);

    // Block signals to avoid possible chaos
    ui->trustee_view->selectionModel()->blockSignals(true);
    load_sd(ad, sd);

    for (PermissionsWidget *permission_widget : permissions_widgets) {
        permission_widget->blockSignals(true);
        permission_widget->init(target_class_list, sd);
        if ((!ui->clear_all_button->isEnabled() && permission_widget->there_are_selected_permissions())) {
            ui->clear_all_button->setEnabled(true);
        }
    }

    sddl_view->update(sd);

    tab_edit->edited();

    ui->trustee_view->selectionModel()->blockSignals(false);
    for (PermissionsWidget *permission_widget : permissions_widgets) {
        permission_widget->blockSignals(false);
    }
    on_applied_objs_cmbbox();

    hide_busy_indicator();
}

void SecurityTab::on_more_menu() {
    restore_sd_action->setEnabled(previous_sd != nullptr);
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

    const AdObject target_object = ad.search_object(target, {ATTRIBUTE_SECURITY_DESCRIPTOR});
    if (!target_object.is_empty()) {
        security_descriptor_free(security_tab->previous_sd);
        security_tab->previous_sd = target_object.get_security_descriptor();
    }
    else {
        total_success = false;
    }

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
    load_sd(ad, sd);

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
