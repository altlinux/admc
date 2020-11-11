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

#include "find_dialog.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "settings.h"
#include "containers_widget.h"
#include "utils.h"
#include "filter.h"
#include "filter_widget/filter_widget.h"
#include "object_list_widget.h"

#include <QString>
#include <QList>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QPushButton>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QDebug>

FindDialog::FindDialog()
: QDialog()
{
    setWindowTitle(tr("Find dialog"));
    resize(600, 600);
    setAttribute(Qt::WA_DeleteOnClose);

    filter_widget = new FilterWidget();

    auto find_button = new QPushButton(tr("Find"));

    find_results = new ObjectListWidget();

    const auto layout = new QVBoxLayout(this);
    layout->addWidget(filter_widget);
    layout->addWidget(find_button);
    layout->addWidget(find_results);

    connect(
        find_button, &QPushButton::clicked,
        this, &FindDialog::find);
}

void FindDialog::find() {
    const QString filter = filter_widget->get_filter();
    const QString search_base = filter_widget->get_search_base();

    find_results->load_filter(filter, search_base);
}
