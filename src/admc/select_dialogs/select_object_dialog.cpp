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

#include "select_object_dialog.h"
#include "ui_select_object_dialog.h"

#include "adldap.h"
#include "console_impls/object_impl.h"
#include "globals.h"
#include "select_object_advanced_dialog.h"
#include "select_object_match_dialog.h"
#include "settings.h"
#include "utils.h"

#include <QStandardItemModel>

enum SelectColumn {
    SelectColumn_Name,
    SelectColumn_Type,
    SelectColumn_Folder,
    SelectColumn_COUNT,
};

SelectObjectDialog::SelectObjectDialog(const QList<QString> class_list_arg, const SelectObjectDialogMultiSelection multi_selection_arg, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::SelectObjectDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    class_list = class_list_arg;
    multi_selection = multi_selection_arg;

    const QList<QString> selected_list = class_list;

    ui->select_classes_widget->set_classes(class_list, selected_list);

    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels(header_labels());

    ui->view->setModel(model);

    enable_widget_on_selection(ui->remove_button, ui->view);

    settings_setup_dialog_geometry(SETTING_select_object_dialog_geometry, this);

    settings_restore_header_state(SETTING_select_object_header_state, ui->view->header());

    connect(
        ui->add_button, &QPushButton::clicked,
        this, &SelectObjectDialog::on_add_button);
    connect(
        ui->remove_button, &QAbstractButton::clicked,
        this, &SelectObjectDialog::on_remove_button);
    connect(
        ui->advanced_button, &QPushButton::clicked,
        this, &SelectObjectDialog::open_advanced_dialog);
}

SelectObjectDialog::~SelectObjectDialog() {
    settings_save_header_state(SETTING_select_object_header_state, ui->view->header());

    delete ui;
}

QList<QString> SelectObjectDialog::header_labels() {
    const QList<QString> out = {
        tr("Name"),
        tr("Type"),
        tr("Folder"),
    };

    return out;
}

QList<QString> SelectObjectDialog::get_selected() const {
    QList<QString> out;

    for (int row = 0; row < model->rowCount(); row++) {
        const QModelIndex index = model->index(row, 0);
        const QString dn = index.data(ObjectRole_DN).toString();

        out.append(dn);
    }

    return out;
}

QList<SelectedObjectData> SelectObjectDialog::get_selected_advanced() const {
    QList<SelectedObjectData> out;

    for (int row = 0; row < model->rowCount(); row++) {
        const QModelIndex index = model->index(row, 0);
        const QString dn = index.data(ObjectRole_DN).toString();
        const QString category = index.data(ObjectRole_ObjectCategory).toString();

        out.append({dn, category});
    }

    return out;
}

void SelectObjectDialog::accept() {
    const QList<QString> selected = get_selected();

    const bool selected_multiple_when_single_selection = (multi_selection == SelectObjectDialogMultiSelection_No && selected.size() > 1);
    if (selected_multiple_when_single_selection) {
        message_box_warning(this, tr("Error"), tr("This selection accepts only one object. Remove extra objects to proceed."));
    } else if (selected.isEmpty()) {
        // TODO: replace with "ok" button turning off
        // if selection is empty. but the
        // "selected_multiple_when_single_selection"
        // should still be done via warning, otherwise
        // would be confusing
        message_box_warning(this, tr("Error"), tr("You must select at least one object."));
    } else {
        QDialog::accept();
    }
}

void SelectObjectDialog::on_add_button() {
    if (ui->name_edit->text().isEmpty()) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    const QString base = ui->select_base_widget->get_base();

    const QString filter = [&]() {
        const QString entered_name = ui->name_edit->text();

        const QString name_filter = filter_OR({
            filter_CONDITION(Condition_StartsWith, ATTRIBUTE_NAME, entered_name),
            filter_CONDITION(Condition_StartsWith, ATTRIBUTE_CN, entered_name),
            filter_CONDITION(Condition_StartsWith, ATTRIBUTE_SAM_ACCOUNT_NAME, entered_name),
            filter_CONDITION(Condition_StartsWith, ATTRIBUTE_USER_PRINCIPAL_NAME, entered_name),
        });

        const QString classes_filter = ui->select_classes_widget->get_filter();

        const QString out = filter_AND({
            name_filter,
            classes_filter,
        });

        return out;
    }();

    const QHash<QString, AdObject> search_results = ad.search(base, SearchScope_All, filter, console_object_search_attributes());

    if (search_results.size() == 1) {
        const QString dn = search_results.keys()[0];
        add_objects_to_list({dn}, ad);
    } else if (search_results.size() > 1) {
        // Open dialog where you can select one of the matches
        auto dialog = new SelectObjectMatchDialog(search_results, this);
        dialog->open();

        connect(
            dialog, &QDialog::accepted,
            this,
            [this, dialog]() {
                const QList<QString> selected_matches = dialog->get_selected();

                add_objects_to_list(selected_matches);
            });
    } else if (search_results.size() == 0) {
        // Warn about failing to find any matches
        message_box_warning(this, tr("Error"), tr("Failed to find any matches."));
    }
}

void SelectObjectDialog::on_remove_button() {
    const QList<QPersistentModelIndex> selected = persistent_index_list(ui->view->selectionModel()->selectedRows());

    for (const QPersistentModelIndex &index : selected) {
        model->removeRows(index.row(), 1);
    }
}

void SelectObjectDialog::open_advanced_dialog() {
    auto dialog = new SelectObjectAdvancedDialog(class_list, this);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog]() {
            const QList<QString> selected = dialog->get_selected_dns();

            add_objects_to_list(selected);
        });
}

void SelectObjectDialog::add_objects_to_list(const QList<QString> &dn_list) {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    add_objects_to_list(dn_list, ad);
}

// Adds objects to the list of selected objects. If list
// contains objects that are already in list, they won't be
// added and a message box will open warning user about
// that.
// NOTE: this is slightly inefficient because we search for
// objects again, when they already were searched for by
// callers of this f-n, in other words we don't reuse search
// results. For majority of cases this is fine. For rare
// cases where user is selecting truly huge amounts of
// objects, they can wait. The alternative of caching
// objects everywhere adds too much complexity.
void SelectObjectDialog::add_objects_to_list(const QList<QString> &dn_list, AdInterface &ad) {
    const QList<QString> current_selected_list = get_selected();

    bool any_duplicates = false;

    for (const QString &dn : dn_list) {
        const bool is_duplicate = current_selected_list.contains(dn);

        if (is_duplicate) {
            any_duplicates = true;
        } else {
            const AdObject object = ad.search_object(dn);

            add_select_object_to_model(model, object);
        }
    }

    if (any_duplicates) {
        message_box_warning(this, tr("Error"), tr("Selected object is already in the list."));
    }

    ui->name_edit->clear();
}

void add_select_object_to_model(QStandardItemModel *model, const AdObject &object) {
    const QList<QStandardItem *> row = make_item_row(SelectColumn_COUNT);

    console_object_item_data_load(row[0], object);

    const QString dn = object.get_dn();
    const QString name = dn_get_name(dn);
    const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);
    const QString type = g_adconfig->get_class_display_name(object_class);
    const QString folder = dn_get_parent_canonical(dn);

    row[SelectColumn_Name]->setText(name);
    row[SelectColumn_Type]->setText(type);
    row[SelectColumn_Folder]->setText(folder);

    model->appendRow(row);
}
