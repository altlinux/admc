/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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
#include "status.h"
#include "globals.h"
#include "console_widget/console_widget.h"
#include "console_types/query.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>
#include <QMessageBox>

EditQueryFolderDialog::EditQueryFolderDialog(ConsoleWidget *console_arg, QWidget *parent)
: QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    console = console_arg;

    const auto title = QString(tr("Edit query folder"));
    setWindowTitle(title);

    const QModelIndex scope_index = get_selected_scope_index(console);
    const QString current_name = scope_index.data(Qt::DisplayRole).toString();
    const QString current_description = scope_index.data(QueryItemRole_Description).toString();

    name_edit = new QLineEdit();
    name_edit->setText(current_name);

    description_edit = new QLineEdit();
    description_edit->setText(current_description);

    auto form_layout = new QFormLayout();

    auto ok_button = new QPushButton(tr("Ok"));

    form_layout->addRow(tr("Name:"), name_edit);
    form_layout->addRow(tr("Description:"), description_edit);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(form_layout);
    layout->addWidget(ok_button);

    connect(
        ok_button, &QAbstractButton::clicked,
        this, &EditQueryFolderDialog::accept);
}

QString EditQueryFolderDialog::get_name() const {
    return name_edit->text();
}

QString EditQueryFolderDialog::get_description() const {
    return description_edit->text();
}

void EditQueryFolderDialog::accept() {
    const QModelIndex scope_index = get_selected_scope_index(console);
    const QModelIndex results_index = console->get_buddy(scope_index);
    const QString name = get_name();
    const QString description = get_description();

    if (!query_name_is_good(name, scope_index.parent(), this, scope_index)) {
        return;
    }

    QStandardItem *scope_item = console->get_scope_item(scope_index);
    const QList<QStandardItem *> results_row = console->get_results_row(results_index);
    load_query_folder(scope_item, results_row, name, description);

    save_queries();

    QDialog::accept();
}
