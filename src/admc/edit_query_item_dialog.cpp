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

#include "edit_query_item_dialog.h"

#include "console_types/console_query.h"
#include "edit_query_item_widget.h"

#include <QLineEdit>
#include <QVBoxLayout>
#include <QDialogButtonBox>

EditQueryItemDialog::EditQueryItemDialog(ConsoleWidget *console_arg)
: QDialog(console_arg)
{
    setAttribute(Qt::WA_DeleteOnClose);
    
    console = console_arg;

    const auto title = QString(tr("Edit Query"));
    setWindowTitle(title);

    edit_query_widget = new EditQueryItemWidget();

    auto buttonbox = new QDialogButtonBox();
    buttonbox->addButton(QDialogButtonBox::Ok);
    buttonbox->addButton(QDialogButtonBox::Cancel);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(edit_query_widget);
    layout->addWidget(buttonbox);

    const QList<QModelIndex> selected_list = console->get_selected_items();

    if (selected_list.isEmpty()) {
        close();

        return;
    }

    index = selected_list[0];

    edit_query_widget->load(index);

    connect(
        buttonbox, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        buttonbox, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
}

void EditQueryItemDialog::accept() {
    QString name;
    QString description;
    QString filter;
    QString search_base;
    QByteArray filter_state;
    edit_query_widget->get_state(name, description, filter, search_base, filter_state);

    const QModelIndex scope_index = console_item_convert_to_scope_index(index);

    if (!console_query_name_is_good(name, scope_index.parent(), this, scope_index)) {
        return;
    }

    QStandardItem *scope_item = console->get_scope_item(scope_index);

    const QModelIndex results_index = console_item_get_buddy(scope_index);
    const QList<QStandardItem *> results_row = console->get_results_row(results_index);

    console_query_item_load(scope_item, results_row, name, description, filter, filter_state, search_base);

    console_query_tree_save(console);

    console->refresh_scope(scope_index);

    QDialog::accept();
}
