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

#include "edit_query_dialog.h"

#include "ad_filter.h"
#include "status.h"
#include "globals.h"
#include "filter_widget/filter_widget.h"
#include "filter_widget/search_base_widget.h"
#include "console_types/console_query.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>
#include <QMessageBox>

EditQueryDialog::EditQueryDialog(ConsoleWidget *console_arg)
: QDialog(console_arg)
{
    setAttribute(Qt::WA_DeleteOnClose);
    
    console = console_arg;

    const auto title = QString(tr("Edit Query"));
    setWindowTitle(title);

    search_base_widget = new SearchBaseWidget();

    filter_widget = new FilterWidget(filter_classes);

    name_edit = new QLineEdit();

    description_edit = new QLineEdit();

    auto form_layout = new QFormLayout();

    auto create_button = new QPushButton(tr("Ok"));

    form_layout->addRow(tr("Name:"), name_edit);
    form_layout->addRow(tr("Description:"), description_edit);
    form_layout->addRow(tr("Search in:"), search_base_widget);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(form_layout);
    layout->addWidget(filter_widget);
    layout->addWidget(create_button);

    const QList<QModelIndex> selected_list = console->get_selected_items();
    // TODO: check empty etc
    index = selected_list[0];

    QByteArray filter_state = index.data(QueryItemRole_FilterState).toByteArray();
    QDataStream filter_state_stream(filter_state);
    filter_state_stream >> search_base_widget;
    filter_state_stream >> filter_widget;

    const QString name = index.data(Qt::DisplayRole).toString();
    name_edit->setText(name);

    const QString description = index.data(QueryItemRole_Description).toString();
    description_edit->setText(description);

    connect(
        create_button, &QAbstractButton::clicked,
        this, &QDialog::accept);
}

void EditQueryDialog::accept() {
    const QString name = name_edit->text();
    const QString description = description_edit->text();
    const QString filter = filter_widget->get_filter();
    const QString search_base = search_base_widget->get_search_base();

    const QByteArray filter_state =
    [&]() {
        QByteArray out;

        QDataStream filter_state_stream(&out, QIODevice::WriteOnly);
        filter_state_stream << search_base_widget;
        filter_state_stream << filter_widget;

        return out;
    }();

    if (!console_query_name_is_good(name, index.parent(), this, index)) {
        return;
    }

    const QModelIndex scope_index = console_item_convert_to_scope_index(index);
    QStandardItem *scope_item = console->get_scope_item(scope_index);

    const QModelIndex results_index = console_item_get_buddy(scope_index);
    const QList<QStandardItem *> results_row = console->get_results_row(results_index);

    console_query_item_load(scope_item, results_row, name, description, filter, filter_state, search_base);

    console_query_tree_save(console);

    console->refresh_scope(scope_index);

    QDialog::accept();
}
