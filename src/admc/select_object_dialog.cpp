/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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
#include "ui_select_object_match_dialog.h"

#include "adldap.h"
#include "console_impls/object_impl.h"
#include "globals.h"
#include "utils.h"
#include "settings.h"
#include "console_impls/object_impl.h"
#include "filter_widget/select_classes_widget.h"
#include "filter_widget/select_base_widget.h"
#include "select_object_advanced_dialog.h"

#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>

enum SelectColumn {
    SelectColumn_Name,
    SelectColumn_Type,
    SelectColumn_Folder,
    SelectColumn_COUNT,
};

void add_select_object_to_model(QStandardItemModel *model, const AdObject &object);

const QList<QString> header_labels = {
    QCoreApplication::translate("SelectObjectDialog", "Name"),        
    QCoreApplication::translate("SelectObjectDialog", "Type"),        
    QCoreApplication::translate("SelectObjectDialog", "Folder"),        
};

SelectObjectDialog::SelectObjectDialog(const QList<QString> class_list_arg, const SelectObjectDialogMultiSelection multi_selection_arg, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::SelectObjectDialog();
    ui->setupUi(this);

    class_list = class_list_arg;
    multi_selection = multi_selection_arg;

    setAttribute(Qt::WA_DeleteOnClose);

    ui->select_classes->add_classes(g_adconfig, class_list);

    ui->select_base_widget->init(g_adconfig);

    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels(header_labels);

    ui->view->setModel(model);

    settings_setup_dialog_geometry(SETTING_select_object_dialog_geometry, this);

    settings_restore_header_state(SETTING_select_object_header_state, ui->view->header());

    connect(
        ui->add_button, &QPushButton::clicked,
        this, &SelectObjectDialog::on_add_button);
    connect(
        ui->advanced_button, &QPushButton::clicked,
        this, &SelectObjectDialog::on_advanced_button);
    connect(
        ui->remove_button, &QAbstractButton::clicked,
        this, &SelectObjectDialog::on_remove_button);
}

SelectObjectDialog::~SelectObjectDialog() {
    settings_save_header_state(SETTING_select_object_header_state, ui->view->header());   
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

void SelectObjectDialog::accept() {
    const QList<QString> selected = get_selected();

    const bool selected_multiple_when_single_selection = (multi_selection == SelectObjectDialogMultiSelection_No && selected.size() > 1);
    if (selected_multiple_when_single_selection) {
        message_box_warning(this, tr("Error"), tr("This selection accepts only one object. Remove extra objects to proceed."));
    } else {
        QDialog::accept();
    }
}

void SelectObjectDialog::on_add_button() {
    if (ui->name_edit->text().isEmpty()) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QString base = ui->select_base_widget->get_base();

    const QString filter = [&]() {
        const QString entered_name = ui->name_edit->text();

        const QString name_filter = filter_OR({
            filter_CONDITION(Condition_StartsWith, ATTRIBUTE_NAME, entered_name),
            filter_CONDITION(Condition_StartsWith, ATTRIBUTE_CN, entered_name),
            filter_CONDITION(Condition_StartsWith, ATTRIBUTE_SAMACCOUNT_NAME, entered_name),
            filter_CONDITION(Condition_StartsWith, ATTRIBUTE_USER_PRINCIPAL_NAME, entered_name),
        });

        const QString classes_filter = ui->select_classes->get_filter();

        const QString out = filter_AND({
            name_filter,
            classes_filter,
        });

        return out;
    }();

    const QHash<QString, AdObject> search_results = ad.search(base, SearchScope_All, filter, console_object_search_attributes());

    if (search_results.size() == 1) {
        // Add to list
        const AdObject object = search_results.values()[0];

        if (is_duplicate(object)) {
            duplicate_message_box();
        } else {
            add_select_object_to_model(model, object);

            ui->name_edit->clear();
        }
    } else if (search_results.size() > 1) {
        // Open dialog where you can select one of the matches
        // TODO: probably make a separate file, decent sized dialog
        auto dialog = new SelectObjectMatchDialog(search_results, this);
        connect(
            dialog, &QDialog::accepted,
            [=]() {
                const QList<QString> selected_list = dialog->get_selected();

                bool any_duplicates = false;

                // TODO: duplicating section above
                for (const QString &dn : selected_list) {
                    const AdObject object = search_results[dn];

                    if (is_duplicate(object)) {
                        any_duplicates = true;
                    } else {
                        add_select_object_to_model(model, object);
                    }
                }

                if (any_duplicates) {
                    duplicate_message_box();
                }

                ui->name_edit->clear();
            });

        dialog->open();
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

void SelectObjectDialog::on_advanced_button() {
    auto dialog = new SelectObjectAdvancedDialog(class_list, this);

    // TODO: can optimize if dialog returns objects
    // directly, but will need to keep them around
    connect(
        dialog, &SelectObjectAdvancedDialog::accepted,
        [=]() {
            AdInterface ad;
            if (ad_failed(ad)) {
                return;
            }

            // NOTE: this is slightly inefficient because
            // we're searching for objects again instead of
            // using the existing data from find results
            // (inside dialog). Using existing data will be
            // more complex though, but if you need
            // performance in the future, do it. Not that
            // big of a deal at the moment because select
            // object dialog is mostly used for small number
            // of objects.
            const QList<QString> selected = dialog->get_selected_dns();

            bool any_duplicates = false;

            for (const QString &dn : selected) {
                const AdObject object = ad.search_object(dn);

                if (is_duplicate(object)) {
                    any_duplicates = true;
                } else {
                    add_select_object_to_model(model, object);
                }
            }

            if (any_duplicates) {
                duplicate_message_box();
            }
        });

    dialog->open();
}

bool SelectObjectDialog::is_duplicate(const AdObject &object) const {
    const QList<QString> selected = get_selected();

    return selected.contains(object.get_dn());
}

void SelectObjectDialog::duplicate_message_box() {
    message_box_warning(this, tr("Error"), tr("Selected object is already in the list."));
}

SelectObjectMatchDialog::SelectObjectMatchDialog(const QHash<QString, AdObject> &search_results, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::SelectObjectMatchDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    auto model = new QStandardItemModel(this);

    model->setHorizontalHeaderLabels(header_labels);

    for (const AdObject &object : search_results) {
        add_select_object_to_model(model, object);
    }

    ui->view->sortByColumn(0, Qt::AscendingOrder);
    ui->view->setModel(model);
}

QList<QString> SelectObjectMatchDialog::get_selected() const {
    QList<QString> out;

    const QList<QModelIndex> selected_indexes = ui->view->selectionModel()->selectedRows();

    for (const QModelIndex &index : selected_indexes) {
        const QString dn = index.data(ObjectRole_DN).toString();
        out.append(dn);
    }

    return out;
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
