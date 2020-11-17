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
#include "object_model.h"
#include "object_context_menu.h"
#include "details_dialog.h"
#include "utils.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "filter.h"

#include <QTreeView>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QGridLayout>

ObjectListWidget::ObjectListWidget()
: QWidget()
{   
    const QList<QString> base_columns = {
        ATTRIBUTE_NAME,
        ATTRIBUTE_OBJECT_CLASS,
        ATTRIBUTE_DESCRIPTION,
        ATTRIBUTE_DISTINGUISHED_NAME
    };
    const QList<QString> extra_columns = ADCONFIG()->get_extra_columns();
    columns = base_columns + extra_columns;

    model = new ObjectModel(columns.count(), column_index(ATTRIBUTE_DISTINGUISHED_NAME), this);

    const QList<QString> header_labels =
    [this]() {
        QList<QString> out;
        for (const QString attribute : columns) {
            const QString attribute_name = ADCONFIG()->get_attribute_display_name(attribute, CLASS_DEFAULT);

            out.append(attribute_name);
        }
        return out;
    }();
    model->setHorizontalHeaderLabels(header_labels);

    auto proxy_name = new QSortFilterProxyModel(this);
    proxy_name->setFilterKeyColumn(column_index(ATTRIBUTE_NAME));

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

    proxy_name->setSourceModel(model);
    view->setModel(proxy_name);

    DetailsDialog::connect_to_open_by_double_click(view, column_index(ATTRIBUTE_DISTINGUISHED_NAME));

    setup_column_toggle_menu(view, model, 
    {
        column_index(ATTRIBUTE_NAME),
        column_index(ATTRIBUTE_OBJECT_CLASS),
        column_index(ATTRIBUTE_DESCRIPTION)
    });

    label = new QLabel(this);

    const auto filter_name_label = new QLabel(tr("Filter: "), this);
    auto filter_name_edit = new QLineEdit(this);

    const auto layout = new QGridLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setColumnStretch(1, 1);

    layout->addWidget(label, 0, 0);
    layout->addWidget(filter_name_label, 0, 2, Qt::AlignRight);
    layout->addWidget(filter_name_edit, 0, 3);
    layout->addWidget(view, 1, 0, 2, 4);

    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &ObjectListWidget::on_context_menu);

    connect(
        filter_name_edit, &QLineEdit::textChanged,
        [proxy_name](const QString &text) {
            proxy_name->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive, QRegExp::FixedString));
        });
    filter_name_edit->setText("");
}

void ObjectListWidget::load_children(const QString &new_parent_dn) {
    parent_dn = new_parent_dn;

    const QList<QString> search_attributes = columns;
    const QString filter = current_advanced_view_filter();
    const QHash<QString, AdObject> search_results = AD()->search(filter, search_attributes, SearchScope_Children, parent_dn);

    load(search_results);
}

void ObjectListWidget::load_filter(const QString &filter, const QString &search_base) {
    const QList<QString> search_attributes = columns;
    const QString filter_and_advanced = filter_AND({filter, current_advanced_view_filter()});
    const QHash<QString, AdObject> search_results = AD()->search(filter_and_advanced, search_attributes, SearchScope_All, search_base);

    load(search_results);
}


void ObjectListWidget::on_context_menu(const QPoint pos) {
    const QString dn =
    [this, pos]() {
        const int dn_column = column_index(ATTRIBUTE_DISTINGUISHED_NAME);
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

    ObjectContextMenu context_menu(dn);
    exec_menu_from_view(&context_menu, view, pos);
}

void ObjectListWidget::load(const QHash<QString, AdObject> &objects) {
    model->removeRows(0, model->rowCount());

    for (auto child_dn : objects.keys()) {
        const AdObject object  = objects[child_dn];
        
        const QList<QStandardItem *> row = make_item_row(columns.count());
        for (int i = 0; i < columns.count(); i++) {
            const QString attribute = columns[i];

            if (!object.contains(attribute)) {
                continue;
            }

            const QString display_value =
            [attribute, object]() {
                if (attribute == ATTRIBUTE_OBJECT_CLASS) {
                    const QString value_string = object.get_string(attribute);
                    return ADCONFIG()->get_class_display_name(value_string);
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

    view->sortByColumn(column_index(ATTRIBUTE_NAME), Qt::AscendingOrder);

    resize_columns();

    const QString label_text =
    [this]() {
        const int object_count = model->rowCount();
        const QString object_count_string = tr("%n object(s)", "", object_count);

        if (!parent_dn.isEmpty()) {
            // "Parent: # objects"
            const QString parent_rdn = dn_get_rdn(parent_dn);

            return QString("%1: %2").arg(parent_rdn, object_count_string);
        } else {
            // "# objects"
            return object_count_string;
        }
    }();

    label->setText(label_text);
}

// Resize name and class columns to fit their contents but
// limit both to a portion of total available width because
// contents can be extremely long
void ObjectListWidget::resize_columns() {
    const int name_column = column_index(ATTRIBUTE_NAME);
    const int class_column = column_index(ATTRIBUTE_OBJECT_CLASS);

    view->resizeColumnToContents(name_column);
    view->resizeColumnToContents(class_column);
    
    const int name_contents_width = view->columnWidth(name_column);
    const int class_contents_width = view->columnWidth(class_column);

    const int max_width = (int) (view->width() * 0.4);
    view->setColumnWidth(name_column, qMin(name_contents_width, max_width));
    view->setColumnWidth(class_column, qMin(class_contents_width, max_width));
}

void ObjectListWidget::showEvent(QShowEvent *event) {
    resize_columns();
}

int ObjectListWidget::column_index(const QString &attribute) {
    if (!columns.contains(attribute)) {
        printf("ObjectListWidget is missing column for %s\n", qPrintable(attribute));
    }

    return columns.indexOf(attribute);
}
