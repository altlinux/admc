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

#include "edit_query_folder_dialog.h"
#include "ui_edit_query_folder_dialog.h"

#include "console_impls/query_folder_impl.h"
#include "settings.h"

EditQueryFolderDialog::EditQueryFolderDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::EditQueryFolderDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    settings_setup_dialog_geometry(SETTING_edit_query_folder_dialog_geometry, this);
}

EditQueryFolderDialog::~EditQueryFolderDialog() {
    delete ui;
}

QString EditQueryFolderDialog::name() const {
    return ui->name_edit->text();
}

QString EditQueryFolderDialog::description() const {
    return ui->description_edit->text();
}

void EditQueryFolderDialog::set_data(const QList<QString> &sibling_name_list_arg, const QString &name, const QString &description) {
    sibling_name_list = sibling_name_list_arg;
    ui->name_edit->setText(name);
    ui->description_edit->setText(description);
}

void EditQueryFolderDialog::accept() {
    const QString name = ui->name_edit->text().trimmed();

    const bool name_is_valid = console_query_or_folder_name_is_good(name, sibling_name_list, this);

    if (name_is_valid) {
        QDialog::accept();
    }
}
