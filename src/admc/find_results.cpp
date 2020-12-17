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
#include "object_context_menu.h"
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

#include <QTreeView>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>
#include <QStandardItemModel>
#include <QVBoxLayout>

FindResults::FindResults()
: QWidget()
{   
    model = new QStandardItemModel(ADCONFIG()->get_columns().count(), ADCONFIG()->get_column_index(ATTRIBUTE_DN), this);

    const QList<QString> header_labels = object_model_header_labels();
    model->setHorizontalHeaderLabels(header_labels);

    view = new QTreeView(this);
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setRootIsDecorated(false);
    view->setItemsExpandable(false);
    view->setExpandsOnDoubleClick(false);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);
    view->header()->setSectionsMovable(true);

    view->setModel(model);

    DetailsDialog::connect_to_open_by_double_click(view, ADCONFIG()->get_column_index(ATTRIBUTE_DN));

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

    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &FindResults::on_context_menu);
}

void FindResults::on_context_menu(const QPoint pos) {
    const QString dn = get_dn_from_pos(pos, view, ADCONFIG()->get_column_index(ATTRIBUTE_DN));
    if (dn.isEmpty()) {
        return;
    }    

    ObjectContextMenu context_menu(dn, view);
    exec_menu_from_view(&context_menu, view, pos);
}

void FindResults::load(const QString &filter, const QString &search_base) {
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

void FindResults::showEvent(QShowEvent *event) {
    resize_columns(view,
    {
        {ADCONFIG()->get_column_index(ATTRIBUTE_NAME), 0.4},
        {ADCONFIG()->get_column_index(ATTRIBUTE_OBJECT_CLASS), 0.4},
    });
}
