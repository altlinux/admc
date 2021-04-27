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

#include "tabs/security_tab.h"

#include "adldap.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QTreeView>
#include <QStandardItemModel>
#include <QLabel>

SecurityTab::SecurityTab() {
    model = new QStandardItemModel(0, 1, this);
    
    view = new QTreeView(this);
    view->setHeaderHidden(true);
    view->setModel(model);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->sortByColumn(0, Qt::AscendingOrder);
    view->setSortingEnabled(true);

    selected_trustee_label = new QLabel();

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);
    layout->addWidget(selected_trustee_label);

    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &SecurityTab::on_selected_trustee_changed);
}

void SecurityTab::load(AdInterface &ad, const AdObject &object) {
    model->removeRows(0, model->rowCount());

    const QList<QString> trustee_list = ad.get_trustee_list(object);
    for (const QString &trustee : trustee_list) {
        auto item = new QStandardItem();
        item->setText(trustee);
        model->appendRow(item);
    }

    // Select first index
    view->selectionModel()->setCurrentIndex(model->index(0, 0), QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);

    model->sort(0, Qt::AscendingOrder);

    PropertiesTab::load(ad, object);
}

void SecurityTab::on_selected_trustee_changed() {
    const QString label_text =
    [&]() {
        const QList<QModelIndex> selected_list = view->selectionModel()->selectedRows();
        
        if (!selected_list.isEmpty()) {
            const QModelIndex selected = selected_list[0];
            const QString selected_name = selected.data(Qt::DisplayRole).toString();
            const QString text = QString("Permissions for %1").arg(selected_name);

            return text;
        } else {
            return QString();
        }
    }();

    selected_trustee_label->setText(label_text);
}
