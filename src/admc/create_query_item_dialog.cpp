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

#include "console_impls/query_item_impl.h"
#include "console_impls/query_folder_impl.h"
#include "edit_query_item_widget.h"
#include "utils.h"
#include "console_impls/item_type.h"

#include <QModelIndex>

CreateQueryItemDialog::CreateQueryItemDialog(ConsoleWidget *console_arg)
: QDialog(console_arg) {
    ui = new Ui::CreateQueryItemDialog();
    ui->setupUi(this);

    console = console_arg;
}

void CreateQueryItemDialog::open() {
    ui->edit_query_widget->load(QModelIndex());

    QDialog::open();
}

void CreateQueryItemDialog::accept() {
    QString name;
    QString description;
    QString filter;
    QString base;
    QByteArray filter_state;
    bool scope_is_children;
    ui->edit_query_widget->save(name, description, filter, base, scope_is_children, filter_state);

    const QModelIndex parent_index = console->get_selected_item(ItemType_QueryItem);

    if (!console_query_or_folder_name_is_good(name, parent_index, this, QModelIndex())) {
        return;
    }

    console_query_item_create(console, name, description, filter, filter_state, base, scope_is_children, parent_index);

    console_query_tree_save(console);

    QDialog::accept();
}
