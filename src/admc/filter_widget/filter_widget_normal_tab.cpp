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

#include "filter_widget/filter_widget_normal_tab.h"
#include "filter_widget/select_classes_widget.h"
#include "filter_widget/filter_builder.h"
#include "ad_interface.h"
#include "filter.h"

#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QDebug>

FilterWidgetNormalTab::FilterWidgetNormalTab()
: FilterWidgetTab()
{
    select_classes = new SelectClassesWidget();

    filter_builder = new FilterBuilder();

    auto add_filter_button = new QPushButton(tr("Add"));
    connect(
        add_filter_button, &QAbstractButton::clicked,
        this, &FilterWidgetNormalTab::on_add_filter);

    filter_list = new QListWidget();

    auto remove_filter_button = new QPushButton(tr("Remove"));
    connect(
        remove_filter_button, &QAbstractButton::clicked,
        this, &FilterWidgetNormalTab::on_remove_filter);

    const QItemSelectionModel *filters_selection_model = filter_list->selectionModel();

    // Enable/disable remove filter button depending on
    // if any filter is currently selected
    connect(
        filters_selection_model, &QItemSelectionModel::selectionChanged,
        [remove_filter_button, filters_selection_model]() {
            const QList<QModelIndex> selecteds = filters_selection_model->selectedIndexes();
            const bool any_selected = !selecteds.isEmpty();

            remove_filter_button->setEnabled(any_selected);
        });
    remove_filter_button->setEnabled(false);

    auto clear_filters_button = new QPushButton(tr("Clear"));
    connect(
        clear_filters_button, &QAbstractButton::clicked,
        this, &FilterWidgetNormalTab::on_clear_filters);

    auto filter_builder_framed = new QFrame();
    filter_builder_framed->setFrameStyle(QFrame::Raised);
    filter_builder_framed->setFrameShape(QFrame::Box);
    {
        auto layout = new QVBoxLayout();
        filter_builder_framed->setLayout(layout);
        layout->addWidget(filter_builder);
        layout->addWidget(add_filter_button);
    }

    auto classes_layout = new QFormLayout();
    classes_layout->addRow(tr("Classes:"), select_classes);

    auto list_buttons_layout = new QHBoxLayout();
    list_buttons_layout->addWidget(remove_filter_button);
    list_buttons_layout->addWidget(clear_filters_button);

    auto layout = new QVBoxLayout();
    setLayout(layout);

    layout->addLayout(classes_layout);

    layout->addWidget(filter_builder_framed);

    layout->addWidget(new QLabel(tr("Filters:")));
    layout->addWidget(filter_list);

    layout->addLayout(list_buttons_layout);
}

QString FilterWidgetNormalTab::get_filter() const {
    const QString attribute_filter =
    [this]() {
        QList<QString> filters;
        for (int i = 0; i < filter_list->count(); i++) {
            const QListWidgetItem *item = filter_list->item(i);
            const QString filter = item->data(Qt::UserRole).toString();

            filters.append(filter);
        }

        return filter_AND(filters);
    }();

    const QString class_filter = select_classes->get_filter();

    const bool classes = !class_filter.isEmpty();
    const bool attributes = !attribute_filter.isEmpty();

    if (classes && attributes) {
        return filter_AND({class_filter, attribute_filter});
    } else if (!classes && attributes) {
        return attribute_filter;
    } else if (classes && !attributes) {
        return class_filter;
    } else {
        return QString();
    }
}

void FilterWidgetNormalTab::on_add_filter() {
    const QString filter = filter_builder->get_filter();
    const QString filter_display = filter_builder->get_filter_display();

    auto item = new QListWidgetItem();
    item->setText(filter_display);
    item->setData(Qt::UserRole, filter);
    filter_list->addItem(item);

    filter_builder->clear();
}

void FilterWidgetNormalTab::on_remove_filter() {
    const QSet<QListWidgetItem *> removed_items =
    [this]() {
        QSet<QListWidgetItem *> out;

        const QItemSelectionModel *selection_model = filter_list->selectionModel();
        const QList<QModelIndex> selecteds = selection_model->selectedIndexes();

        for (const auto selected : selecteds) {
            const int row = selected.row();
            QListWidgetItem *item = filter_list->item(row);

            out.insert(item);
        }

        return out;
    }();

    for (auto item : removed_items) {
        delete item;
    }
}

void FilterWidgetNormalTab::on_clear_filters() {
    filter_list->clear();
}
