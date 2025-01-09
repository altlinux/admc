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

#include "select_object_match_dialog.h"
#include "ui_select_object_match_dialog.h"

#include "console_impls/object_impl.h"
#include "select_object_dialog.h"
#include "settings.h"
#include "utils.h"

#include <QPushButton>
#include <QStandardItemModel>

SelectObjectMatchDialog::SelectObjectMatchDialog(const QHash<QString, AdObject> &search_results, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::SelectObjectMatchDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    model = new QStandardItemModel(this);

    model->setHorizontalHeaderLabels(SelectObjectDialog::header_labels());

    ui->view->sortByColumn(0, Qt::AscendingOrder);
    ui->view->setModel(model);

    for (const AdObject &object : search_results) {
        add_select_object_to_model(model, object);
    }

    QPushButton *ok_button = ui->button_box->button(QDialogButtonBox::Ok);
    enable_widget_on_selection(ok_button, ui->view);

    settings_setup_dialog_geometry(SETTING_select_object_match_dialog_geometry, this);
    settings_restore_header_state(SETTING_select_object_match_header_state, ui->view->header());
}

SelectObjectMatchDialog::~SelectObjectMatchDialog() {
    settings_save_header_state(SETTING_select_object_match_header_state, ui->view->header());

    delete ui;
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
