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

#include "create_query_dialog.h"

#include "ad_filter.h"
#include "status.h"
#include "globals.h"
#include "filter_widget/filter_widget.h"
#include "filter_widget/search_base_widget.h"
#include "console_types/query.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>
#include <QMessageBox>

CreateQueryDialog::CreateQueryDialog(const QModelIndex &parent_index_arg, QWidget *parent)
: QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    parent_index = parent_index_arg;

    const auto title = QString(tr("Create Query"));
    setWindowTitle(title);

    search_base_widget = new SearchBaseWidget();

    filter_widget = new FilterWidget(filter_classes);

    name_edit = new QLineEdit();
    name_edit->setText("New query");

    description_edit = new QLineEdit();

    auto form_layout = new QFormLayout();

    auto create_button = new QPushButton(tr("Create"));

    form_layout->addRow(tr("Name:"), name_edit);
    form_layout->addRow(tr("Description:"), description_edit);
    form_layout->addRow(tr("Search in:"), search_base_widget);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(form_layout);
    layout->addWidget(filter_widget);
    layout->addWidget(create_button);

    connect(
        create_button, &QAbstractButton::clicked,
        this, &QDialog::accept);
}

QString CreateQueryDialog::get_name() const {
    return name_edit->text();
}

QString CreateQueryDialog::get_description() const {
    return description_edit->text();
}

QString CreateQueryDialog::get_filter() const {
    return filter_widget->get_filter();
}

QString CreateQueryDialog::get_search_base() const {
    return search_base_widget->get_search_base();
}

void CreateQueryDialog::accept() {
    const QString name = get_name();
    if (!query_name_is_good(name, parent_index, this, QModelIndex())) {
        return;
    }

    QDialog::accept();
}
