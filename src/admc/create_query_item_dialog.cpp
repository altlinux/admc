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

#include "create_query_item_dialog.h"
#include "ui_create_query_item_dialog.h"

#include "console_impls/query_folder_impl.h"

CreateQueryItemDialog::CreateQueryItemDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::CreateQueryItemDialog();
    ui->setupUi(this);
}

CreateQueryItemDialog::~CreateQueryItemDialog() {
    delete ui;
}

EditQueryItemWidget *CreateQueryItemDialog::edit_widget() const {
    return ui->edit_query_widget;
}

void CreateQueryItemDialog::set_sibling_name_list(const QList<QString> &list) {
    sibling_name_list = list;
}

void CreateQueryItemDialog::open() {
    ui->edit_query_widget->clear();

    QDialog::open();
}

void CreateQueryItemDialog::accept() {
    const QString name = ui->edit_query_widget->name();
    const bool name_is_valid = console_query_or_folder_name_is_good(name, sibling_name_list, this);

    if (name_is_valid) {
        QDialog::accept();
    }
}
