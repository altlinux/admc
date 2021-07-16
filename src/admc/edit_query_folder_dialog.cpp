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

#include "edit_query_folder_dialog.h"

#include "ad_filter.h"
#include "console_types/console_query.h"
#include "console_widget/console_widget.h"
#include "globals.h"
#include "status.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QModelIndex>

EditQueryFolderDialog::EditQueryFolderDialog(ConsoleWidget *console_arg)
: QDialog(console_arg) {
    console = console_arg;

    const auto title = QString(tr("Edit Query Folder"));
    setWindowTitle(title);

    name_edit = new QLineEdit();
    description_edit = new QLineEdit();

    auto form_layout = new QFormLayout();

    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);

    form_layout->addRow(tr("Name:"), name_edit);
    form_layout->addRow(tr("Description:"), description_edit);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(form_layout);
    layout->addWidget(ok_button);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
}

void EditQueryFolderDialog::open() {
    const QModelIndex index = console->get_selected_item();
    const QString current_name = index.data(Qt::DisplayRole).toString();
    const QString current_description = index.data(QueryItemRole_Description).toString();

    name_edit->setText(current_name);
    description_edit->setText(current_description);

    QDialog::open();
}

void EditQueryFolderDialog::accept() {
    const QModelIndex index = console->get_selected_item();
    const QString name = name_edit->text();
    const QString description = description_edit->text();

    if (!console_query_or_folder_name_is_good(console, name, index.parent(), this, index)) {
        return;
    }

    const QList<QStandardItem *> row = console->get_row(index);
    console_query_folder_load(row, name, description);

    console_query_tree_save(console);

    QDialog::accept();
}
