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

#include "find_results.h"
#include "object_menu.h"
#include "details_dialog.h"
#include "utils.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "ad_utils.h"
#include "ad_object.h"
#include "attribute_display.h"
#include "filter.h"
#include "settings.h"
#include "object_model.h"
#include "find_dialog.h"

#include <QTreeView>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>
#include <QStandardItemModel>
#include <QVBoxLayout>

FindResults::FindResults(const FindDialogType type)
: QWidget()
{   
    model = new ObjectModel(this);

    const QList<QString> header_labels = object_model_header_labels();
    model->setHorizontalHeaderLabels(header_labels);

    view = new QTreeView(this);
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->setRootIsDecorated(false);
    view->setItemsExpandable(false);
    view->setExpandsOnDoubleClick(false);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);
    view->header()->setSectionsMovable(true);

    view->setModel(model);

    if (type == FindDialogType_Normal) {
        DetailsDialog::connect_to_open_by_double_click(view, ADCONFIG()->get_column_index(ATTRIBUTE_DN));

        ObjectMenu::setup_as_context_menu(view, ADCONFIG()->get_column_index(ATTRIBUTE_DN));
    }

    setup_column_toggle_menu(view, model, 
    {
        ADCONFIG()->get_column_index(ATTRIBUTE_NAME),
        ADCONFIG()->get_column_index(ATTRIBUTE_OBJECT_CLASS),
        ADCONFIG()->get_column_index(ATTRIBUTE_DESCRIPTION)
    });

    object_count_label = new QLabel();

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(object_count_label);
    layout->addWidget(view);
}

void FindResults::load(const QString &filter, const QString &search_base) {
    object_count_label->clear();

    model->removeRows(0, model->rowCount());
    
    const QList<QString> search_attributes = ADCONFIG()->get_columns();
    const QString filter_and_advanced = filter_AND({filter, current_advanced_view_filter()});
    const QHash<QString, AdObject> search_results = AD()->search(filter_and_advanced, search_attributes, SearchScope_All, search_base);

    for (const AdObject object : search_results) {
        const QList<QStandardItem *> row = make_item_row(ADCONFIG()->get_columns().count());

        load_attributes_row(row, object);

        model->appendRow(row);
    }

    view->sortByColumn(ADCONFIG()->get_column_index(ATTRIBUTE_NAME), Qt::AscendingOrder);

    const QString label_text = tr("%n object(s)", "", model->rowCount());
    object_count_label->setText(label_text);
}

QList<QList<QStandardItem *>> FindResults::get_selected_rows() const {
    const QList<QModelIndex> selected_rows = view->selectionModel()->selectedRows();

    QList<QList<QStandardItem *>> out;

    for (const QModelIndex row_index : selected_rows) {
        const int row = row_index.row();

        QList<QStandardItem *> row_copy;

        for (int col = 0; col < model->columnCount(); col++) {
            QStandardItem *item = model->item(row, col);
            QStandardItem *item_copy = item->clone();
            row_copy.append(item_copy);
        }

        out.append(row_copy);
    }

    return out;
}

void FindResults::showEvent(QShowEvent *event) {
    resize_columns(view,
    {
        {ADCONFIG()->get_column_index(ATTRIBUTE_NAME), 0.4},
        {ADCONFIG()->get_column_index(ATTRIBUTE_OBJECT_CLASS), 0.4},
    });
}
