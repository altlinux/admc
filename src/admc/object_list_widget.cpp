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

#include "object_list_widget.h"
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

#include <QTreeView>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QGridLayout>
#include <QStandardItemModel>

ObjectListWidget::ObjectListWidget(const ObjectListWidgetType list_type_arg)
: QWidget()
{   
    list_type = list_type_arg;

    model = new QStandardItemModel(ADCONFIG()->get_columns().count(), ADCONFIG()->get_column_index(ATTRIBUTE_DISTINGUISHED_NAME), this);

    const QList<QString> header_labels =
    [this]() {
        QList<QString> out;
        for (const QString attribute : ADCONFIG()->get_columns()) {
            const QString attribute_display_name = ADCONFIG()->get_column_display_name(attribute);

            out.append(attribute_display_name);
        }
        return out;
    }();
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

    DetailsDialog::connect_to_open_by_double_click(view, ADCONFIG()->get_column_index(ATTRIBUTE_DISTINGUISHED_NAME));

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
        this, &ObjectListWidget::on_context_menu);
}

void ObjectListWidget::load_children(const QString &new_parent_dn, const QString &filter) {
    parent_dn = new_parent_dn;

    const QList<QString> search_attributes = QList<QString>();
    const QString total_filter = filter_AND({filter, current_advanced_view_filter()});
    const QHash<QString, AdObject> search_results = AD()->search(total_filter, search_attributes, SearchScope_Children, parent_dn);

    load(search_results);
}

void ObjectListWidget::load_filter(const QString &filter, const QString &search_base) {
    const QList<QString> search_attributes = ADCONFIG()->get_columns();
    const QString filter_and_advanced = filter_AND({filter, current_advanced_view_filter()});
    const QHash<QString, AdObject> search_results = AD()->search(filter_and_advanced, search_attributes, SearchScope_All, search_base);

    load(search_results);
}

void ObjectListWidget::on_context_menu(const QPoint pos) {
    const QString dn =
    [this, pos]() {
        const int dn_column = ADCONFIG()->get_column_index(ATTRIBUTE_DISTINGUISHED_NAME);
        QString out = get_dn_from_pos(pos, view, dn_column);
        
        // Interpret clicks on empty space as clicks on parent (if parent is defined
        if (out.isEmpty() && !parent_dn.isEmpty()) {
            out = parent_dn;
        }

        return out;
    }();
    if (dn.isEmpty()) {
        return;
    }    

    ObjectContextMenu context_menu(dn, view);
    exec_menu_from_view(&context_menu, view, pos);
}

void ObjectListWidget::load(const QHash<QString, AdObject> &objects) {
    model->removeRows(0, model->rowCount());

    for (auto child_dn : objects.keys()) {
        const AdObject object  = objects[child_dn];
        
        const QList<QStandardItem *> row = make_item_row(ADCONFIG()->get_columns().count());
        for (int i = 0; i < ADCONFIG()->get_columns().count(); i++) {
            const QString attribute = ADCONFIG()->get_columns()[i];

            if (!object.contains(attribute)) {
                continue;
            }

            const QString display_value =
            [attribute, object]() {
                if (attribute == ATTRIBUTE_OBJECT_CLASS) {
                    const QString object_class = object.get_string(attribute);

                    if (object_class == CLASS_GROUP) {
                        const GroupScope scope = object.get_group_scope(); 
                        const QString scope_string = group_scope_string(scope);

                        const GroupType type = object.get_group_type(); 
                        const QString type_string = group_type_string(type);

                        return QString("%1 Group - %2").arg(type_string, scope_string);
                    } else {
                        return ADCONFIG()->get_class_display_name(object_class);
                    }
                } else {
                    const QByteArray value = object.get_value(attribute);
                    return attribute_display_value(attribute, value);
                }
            }();

            row[i]->setText(display_value);
        }

        const QIcon icon = object.get_icon();
        row[0]->setIcon(icon);

        model->appendRow(row);
    }

    view->sortByColumn(ADCONFIG()->get_column_index(ATTRIBUTE_NAME), Qt::AscendingOrder);

    const QString label_text = tr("%n object(s)", "", model->rowCount());
    object_count_label->setText(label_text);
}

void ObjectListWidget::showEvent(QShowEvent *event) {
    resize_columns(view,
    {
        {ADCONFIG()->get_column_index(ATTRIBUTE_NAME), 0.4},
        {ADCONFIG()->get_column_index(ATTRIBUTE_OBJECT_CLASS), 0.4},
    });
}
