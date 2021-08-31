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

#include "ad_filter.h"
#include "console_types/console_query.h"
#include "filter_widget/filter_widget.h"
#include "filter_widget/select_base_widget.h"
#include "globals.h"
#include "status.h"
#include "item_type.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QModelIndex>

CreateQueryFolderDialog::CreateQueryFolderDialog(ConsoleWidget *console_arg)
: QDialog(console_arg) {
    setAttribute(Qt::WA_DeleteOnClose);
    
    console = console_arg;

    setWindowTitle(tr("Create Query Folder"));

    name_edit = new QLineEdit();
    name_edit->setText("New folder");

    description_edit = new QLineEdit();

    auto button_box = new QDialogButtonBox();
    button_box->addButton(tr("Create"), QDialogButtonBox::AcceptRole);
    button_box->addButton(QDialogButtonBox::Cancel);

    auto form_layout = new QFormLayout();
    form_layout->addRow(tr("Name:"), name_edit);
    form_layout->addRow(tr("Description:"), description_edit);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(form_layout);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
}

void CreateQueryFolderDialog::open() {
    name_edit->setText(tr("New folder"));
    description_edit->setText("");

    QDialog::open();
}

void CreateQueryFolderDialog::accept() {
    const QModelIndex parent_index = console->get_selected_item(ItemType_QueryFolder);
    const QString name = name_edit->text();
    const QString description = description_edit->text();

    if (!console_query_or_folder_name_is_good(name, parent_index, this, QModelIndex())) {
        return;
    }

    console_query_folder_create(console, name, description, parent_index);

    console_query_tree_save(console);

    QDialog::accept();
}
