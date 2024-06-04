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

#include "ad_security.h"
#include "adldap.h"
#include "globals.h"
#include "select_dialogs/select_object_dialog.h"
#include "select_well_known_trustee_dialog.h"
#include "settings.h"
#include "utils.h"

#include "samba/ndr_security.h"

#include <QDebug>
#include <QLabel>
#include <QPersistentModelIndex>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

struct security_descriptor;

enum TrusteeItemRole {
    TrusteeItemRole_Sid = Qt::UserRole,
};

enum EditAction {
    EditAction_None,
    EditAction_Add,
    EditAction_Remove,
};

enum RightsItemRole {
    RightsItemRole_AccessMask = Qt::UserRole,
    RightsItemRole_ObjectType,
    RightsItemRole_ObjectTypeName,
};

class RightsSortModel final : public QSortFilterProxyModel {
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override {
        const QString name_left = source_left.data(Qt::DisplayRole).toString();
        const QString name_right = source_right.data(Qt::DisplayRole).toString();
        const QString object_type_name_left = source_left.data(RightsItemRole_ObjectTypeName).toString();
        const QString object_type_name_right = source_right.data(RightsItemRole_ObjectTypeName).toString();
        const QByteArray object_type_left = source_left.data(RightsItemRole_ObjectType).toByteArray();
        const QByteArray object_type_right = source_right.data(RightsItemRole_ObjectType).toByteArray();
        const uint32_t access_mask_left = source_left.data(RightsItemRole_AccessMask).toUInt();
        const uint32_t access_mask_right = source_right.data(RightsItemRole_AccessMask).toUInt();
        const bool is_common_left = object_type_left.isEmpty();
        const bool is_common_right = object_type_right.isEmpty();
        const bool is_control_left = (access_mask_left == SEC_ADS_CONTROL_ACCESS);
        const bool is_control_right = (access_mask_right == SEC_ADS_CONTROL_ACCESS);
        const bool is_read_left = (access_mask_left == SEC_ADS_READ_PROP);
        const bool is_read_right = (access_mask_right == SEC_ADS_READ_PROP);

        // Generic are before non-generic
        if (is_common_left != is_common_right) {
            return is_common_left;
        }

        // Generic among generic are in pre-defined order
        if (is_common_left && is_common_right) {
            const int common_index_left = common_rights_list.indexOf(access_mask_left);
            const int common_index_right = common_rights_list.indexOf(access_mask_right);

            return (common_index_left < common_index_right);
        }

        // Control rights are before read/write rights
        if (is_control_left != is_control_right) {
            return is_control_left;
        }

        // Control rights are sorted by name
        if (is_control_left && is_control_right) {
            return name_left < name_right;
        }

        // Read/write rights are sorted by name
        if (object_type_left != object_type_right) {
            return object_type_name_left < object_type_name_right;
        }

        // Read rights are before write rights
        if (is_read_left != is_read_right) {
            return is_read_left;
        }

        return name_left < name_right;
    }
};

SecurityTab::SecurityTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::SecurityTab();
    ui->setupUi(this);

    tab_edit = new SecurityTabEdit(ui, this);

    edit_list->append({
        tab_edit,
    });
}

void SecurityTab::fix_acl_order() {
    tab_edit->fix_acl_order();
}

void SecurityTab::set_read_only() {
    tab_edit->set_read_only();
}

bool SecurityTab::verify_acl_order() const {
    return tab_edit->verify_acl_order();
}

SecurityTabEdit::SecurityTabEdit(Ui::SecurityTab *ui_arg, QObject *parent)
: AttributeEdit(parent) {
    ui = ui_arg;

    sd = nullptr;

    ignore_item_changed_signal = false;
    read_only = false;

    trustee_model = new QStandardItemModel(0, 1, this);

    ui->trustee_view->setModel(trustee_model);

    rights_model = new QStandardItemModel(0, AceColumn_COUNT, this);
    set_horizontal_header_labels_from_map(rights_model,
        {
            {AceColumn_Name, tr("Name")},
            {AceColumn_Allowed, tr("Allowed")},
            {AceColumn_Denied, tr("Denied")},
        });

    rights_sort_model = new RightsSortModel(this);
    rights_sort_model->setSourceModel(rights_model);

    ui->rights_view->setModel(rights_sort_model);
    ui->rights_view->setColumnWidth(AceColumn_Name, 400);

    settings_restore_header_state(SETTING_security_tab_header_state, ui->rights_view->header());

    connect(
        ui->trustee_view->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &SecurityTabEdit::load_rights_model);
    connect(
        rights_model, &QStandardItemModel::itemChanged,
        this, &SecurityTabEdit::on_item_changed);
    connect(
        ui->add_trustee_button, &QAbstractButton::clicked,
        this, &SecurityTabEdit::on_add_trustee_button);
    connect(
        ui->add_well_known_trustee_button, &QAbstractButton::clicked,
        this, &SecurityTabEdit::on_add_well_known_trustee);
    connect(
        ui->remove_trustee_button, &QAbstractButton::clicked,
        this, &SecurityTabEdit::on_remove_trustee_button);
}

SecurityTabEdit::~SecurityTabEdit() {
    if (sd != nullptr) {
        security_descriptor_free(sd);
    }
}

void SecurityTabEdit::fix_acl_order() {
    security_descriptor_sort_dacl(sd);

    emit edited();
}

void SecurityTabEdit::set_read_only() {
    ui->add_trustee_button->setEnabled(false);
    ui->add_well_known_trustee_button->setEnabled(false);
    ui->remove_trustee_button->setEnabled(false);

    make_rights_model_read_only();

    read_only = true;
}

bool SecurityTabEdit::verify_acl_order() const {
    const bool out = security_descriptor_verify_acl_order(sd);

    return out;
}

SecurityTab::~SecurityTab() {
    settings_save_header_state(SETTING_security_tab_header_state, ui->rights_view->header());

    delete ui;
}

// Load sd data into trustee model and rights model
// NOTE: load_rights_model() is not explicitly called
// here but it is called implicitly because
// setCurrentIndex() emits currentChanged() signal
// which calls load_rights_model()
void SecurityTabEdit::load_current_sd(AdInterface &ad) {
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
    security_descriptor_free(sd);
    sd = object.get_security_descriptor();

    target_class_list = object.get_strings(ATTRIBUTE_OBJECT_CLASS);

    // Create items in rights model. These will not
    // change until target object changes. Only the
    // state of items changes during editing.
    const QList<SecurityRight> right_list = ad_security_get_right_list_for_class(g_adconfig, target_class_list);

    rights_model->removeRows(0, rights_model->rowCount());

    const QLocale::Language language = []() {
        const QLocale saved_locale = settings_get_variant(SETTING_locale).toLocale();
        const QLocale::Language out = saved_locale.language();

        return out;
    }();

    for (const SecurityRight &right : right_list) {
        const QList<QStandardItem *> row = make_item_row(AceColumn_COUNT);

        // TODO: for russian, probably do "read/write
        // property - [property name]" to avoid having
        // to do suffixes properties
        const QString right_name = ad_security_get_right_name(g_adconfig, right.access_mask, right.object_type, language);

        const QString object_type_name = g_adconfig->get_right_name(right.object_type, language);

        row[AceColumn_Name]->setText(right_name);
        row[AceColumn_Allowed]->setCheckable(true);
        row[AceColumn_Denied]->setCheckable(true);

        row[0]->setData(right.access_mask, RightsItemRole_AccessMask);
        row[0]->setData(right.object_type, RightsItemRole_ObjectType);
        row[0]->setData(object_type_name, RightsItemRole_ObjectTypeName);

        rights_model->appendRow(row);
    }

    // NOTE: because rights model is dynamically filled
    // when trustee switches, we have to make rights
    // model read only again after it's reloaded
    if (read_only) {
        make_rights_model_read_only();
    }

    rights_sort_model->sort(0);

    load_current_sd(ad);

    is_policy = object.is_class(CLASS_GP_CONTAINER);
}

// Load rights model based on current sd and current
// trustee
void SecurityTabEdit::load_rights_model() {
    const QModelIndex current_index = ui->trustee_view->currentIndex();
    if (!current_index.isValid()) {
        return;
    }

    // NOTE: this flag is turned on so that
    // on_item_changed() slot doesn't react to us
    // changing state of items
    ignore_item_changed_signal = true;

    const QByteArray trustee = get_current_trustee();

    for (int row = 0; row < rights_model->rowCount(); row++) {
        const SecurityRightState state = [&]() {
            QStandardItem *item = rights_model->item(row, 0);
            const uint32_t access_mask = item->data(RightsItemRole_AccessMask).toUInt();

            const QByteArray object_type = item->data(RightsItemRole_ObjectType).toByteArray();
            const SecurityRightState out = security_descriptor_get_right(sd, trustee, access_mask, object_type);

            return out;
        }();

        const QHash<SecurityRightStateType, QStandardItem *> item_map = {
            {SecurityRightStateType_Allow, rights_model->item(row, AceColumn_Allowed)},
            {SecurityRightStateType_Deny, rights_model->item(row, AceColumn_Denied)},
        };

        for (int type_i = 0; type_i < SecurityRightStateType_COUNT; type_i++) {
            const SecurityRightStateType type = (SecurityRightStateType) type_i;

            QStandardItem *item = item_map[type];

            const bool object_state = state.get(SecurityRightStateInherited_No, type);
            const bool inherited_state = state.get(SecurityRightStateInherited_Yes, type);

            // Checkboxes become disabled if they
            // contain only inherited state. Note that
            // if there's both inherited and object
            // state for same right, checkbox is
            // enabled so that user can remove object
            // state.
            const bool disabled = (inherited_state && !object_state);
            item->setEnabled(!disabled);

            const Qt::CheckState check_state = [&]() {
                if (object_state || inherited_state) {
                    return Qt::Checked;
                } else {
                    return Qt::Unchecked;
                }
            }();
            item->setCheckState(check_state);
        }
    }

    // NOTE: need to make read only again because
    // during load, items are enabled/disabled based on
    // their inheritance state
    if (read_only) {
        make_rights_model_read_only();
    }

    ignore_item_changed_signal = false;
}

void SecurityTabEdit::make_rights_model_read_only() {
    // NOTE: important to ignore this signal because
    // it's slot reloads the rights model
    ignore_item_changed_signal = true;

    for (int row = 0; row < rights_model->rowCount(); row++) {
        const QList<int> col_list = {
            AceColumn_Allowed,
            AceColumn_Denied,
        };

        for (const int col : col_list) {
            QStandardItem *item = rights_model->item(row, col);
            item->setEnabled(false);
        }
    }

    ignore_item_changed_signal = false;
}

void SecurityTabEdit::on_item_changed(QStandardItem *item) {
    // NOTE: in some cases we need to ignore this signal
    if (ignore_item_changed_signal) {
        return;
    }

    const AceColumn column = (AceColumn) item->column();

    const bool incorrect_column = (column != AceColumn_Allowed && column != AceColumn_Denied);
    if (incorrect_column) {
        return;
    }

    QStandardItem *main_item = rights_model->item(item->row(), 0);

    const bool checked = (item->checkState() == Qt::Checked);

    const QByteArray trustee = get_current_trustee();
    const uint32_t access_mask = main_item->data(RightsItemRole_AccessMask).toUInt();
    const QByteArray object_type = main_item->data(RightsItemRole_ObjectType).toByteArray();
    const bool allow = (column == AceColumn_Allowed);

    if (checked) {
        security_descriptor_add_right(sd, g_adconfig, target_class_list, trustee, access_mask, object_type, allow);
    } else {
        security_descriptor_remove_right(sd, g_adconfig, target_class_list, trustee, access_mask, object_type, allow);
    }

    load_rights_model();

    emit edited();
}

bool SecurityTabEdit::verify(AdInterface &ad, const QString &target) const {
    UNUSED_ARG(target);
    if (is_policy) {
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

    total_success = (total_success && ad_security_replace_security_descriptor(ad, target, sd));

    if (is_policy) {
        total_success = (total_success && ad.gpo_sync_perms(target));
    }

    return total_success;
}

void SecurityTabEdit::on_add_trustee_button() {
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

void SecurityTabEdit::on_remove_trustee_button() {
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
        emit edited();
    }
}

void SecurityTabEdit::add_trustees(const QList<QByteArray> &sid_list, AdInterface &ad) {
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
        emit edited();
    }

    if (failed_to_add_because_already_exists) {
        message_box_warning(ui->trustee_view, tr("Error"), tr("Failed to add some trustee's because they are already in the list."));
    }
}

void SecurityTabEdit::on_add_well_known_trustee() {
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

QByteArray SecurityTabEdit::get_current_trustee() const {
    const QModelIndex current_index = ui->trustee_view->currentIndex();
    QStandardItem *current_item = trustee_model->itemFromIndex(current_index);
    const QByteArray out = current_item->data(TrusteeItemRole_Sid).toByteArray();

    return out;
}
