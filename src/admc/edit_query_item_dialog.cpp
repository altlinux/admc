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

#include "edit_query_item_dialog.h"

#include "console_impls/query_item_impl.h"
#include "console_impls/query_folder_impl.h"
#include "edit_query_item_widget.h"
#include "utils.h"
#include "console_impls/item_type.h"

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QModelIndex>

EditQueryItemDialog::EditQueryItemDialog(ConsoleWidget *console_arg)
: QDialog(console_arg) {
    setAttribute(Qt::WA_DeleteOnClose);

    console = console_arg;

    setWindowTitle(tr("Edit Query"));

    edit_query_widget = new EditQueryItemWidget();

    auto button_box = new QDialogButtonBox();
    button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(edit_query_widget);
    layout->addWidget(button_box);

    const QModelIndex index = console->get_selected_item(ItemType_QueryItem);

    edit_query_widget->load(index);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
}

void EditQueryItemDialog::accept() {
    QString name;
    QString description;
    QString filter;
    QString base;
    QByteArray filter_state;
    bool scope_is_children;
    edit_query_widget->save(name, description, filter, base, scope_is_children, filter_state);

    const QModelIndex index = console->get_selected_item(ItemType_QueryItem);

    if (!console_query_or_folder_name_is_good(name, index.parent(), this, index)) {
        return;
    }

    const QList<QStandardItem *> row = console->get_row(index);

    console_query_item_load(row, name, description, filter, filter_state, base, scope_is_children);

    console_query_tree_save(console);

    console->refresh_scope(index);

    QDialog::accept();
}
