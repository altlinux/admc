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

#include "create_query_folder_dialog.h"
#include "ui_create_query_folder_dialog.h"

#include "ad_filter.h"
#include "console_impls/query_item_impl.h"
#include "console_impls/query_folder_impl.h"
#include "filter_widget/filter_widget.h"
#include "filter_widget/select_base_widget.h"
#include "globals.h"
#include "status.h"
#include "console_impls/item_type.h"

#include <QModelIndex>

CreateQueryFolderDialog::CreateQueryFolderDialog(ConsoleWidget *console_arg)
: QDialog(console_arg) {
    ui = new Ui::CreateQueryFolderDialog();
    ui->setupUi(this);

    console = console_arg;
}

void CreateQueryFolderDialog::open() {
    ui->name_edit->setText(tr("New folder"));
    ui->description_edit->setText("");

    QDialog::open();
}

void CreateQueryFolderDialog::accept() {
    const QModelIndex parent_index = console->get_selected_item(ItemType_QueryFolder);
    const QString name = ui->name_edit->text();
    const QString description = ui->description_edit->text();

    if (!console_query_or_folder_name_is_good(name, parent_index, this, QModelIndex())) {
        return;
    }

    console_query_folder_create(console, name, description, parent_index);

    console_query_tree_save(console);

    QDialog::accept();
}
