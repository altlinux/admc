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

#include "contents_widget.h"
#include "containers_widget.h"
#include "advanced_view_proxy.h"
#include "object_context_menu.h"
#include "details_widget.h"
#include "settings.h"
#include "utils.h"
#include "ad_interface.h"
#include "server_configuration.h"

#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDebug>

// NOTE: not using enums for columns here, because each column maps directly to an attribute
QList<QString> columns;

int column_index(const QString &attribute) {
    if (!columns.contains(attribute)) {
        printf("ContentsWidget is missing column for %s\n", qPrintable(attribute));
    }

    return columns.indexOf(attribute);
}

ContentsWidget::ContentsWidget(ContainersWidget *containers_widget, QWidget *parent)
: QWidget(parent)
{   
    if (columns.isEmpty()) {
        columns = {
            ATTRIBUTE_NAME,
            ATTRIBUTE_OBJECT_CATEGORY,
            ATTRIBUTE_DESCRIPTION
        };
        // NOTE: dn is not one of ADUC's columns, but adding it here for convenience
        columns.append(ATTRIBUTE_DISTINGUISHED_NAME);
        const QList<QString> extra_columns =get_extra_contents_columns();
        columns.append(extra_columns);
    }

    model = new ContentsModel(this);
    const auto advanced_view_proxy = new AdvancedViewProxy(column_index(ATTRIBUTE_DISTINGUISHED_NAME), this);

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
    view->sortByColumn(column_index(ATTRIBUTE_NAME), Qt::AscendingOrder);
    ObjectContextMenu::connect_view(view, column_index(ATTRIBUTE_DISTINGUISHED_NAME));

    setup_model_chain(view, model, {advanced_view_proxy});

    setup_column_toggle_menu(view, model, 
    {
        column_index(ATTRIBUTE_NAME),
        column_index(ATTRIBUTE_OBJECT_CATEGORY),
        column_index(ATTRIBUTE_DESCRIPTION)
    });

    label = new QLabel(this);

    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(label);
    layout->addWidget(view);

    connect(
        containers_widget, &ContainersWidget::selected_changed,
        this, &ContentsWidget::on_containers_selected_changed);

    connect(
        AdInterface::instance(), &AdInterface::modified,
        this, &ContentsWidget::on_ad_modified);

    connect(
        view, &QAbstractItemView::clicked,
        this, &ContentsWidget::on_view_clicked);
}

void ContentsWidget::on_containers_selected_changed(const QString &dn) {
    change_target(dn);
}

void ContentsWidget::on_ad_modified() {
    change_target(target_dn);
}

void ContentsWidget::on_view_clicked(const QModelIndex &index) {
    const bool details_from_contents = Settings::instance()->get_bool(BoolSetting_DetailsFromContents);

    if (details_from_contents) {
        const QString dn = get_dn_from_index(index, column_index(ATTRIBUTE_DISTINGUISHED_NAME));
        DetailsWidget::change_target(dn);
    }
}

void ContentsWidget::change_target(const QString &dn) {
    target_dn = dn;

    model->change_target(target_dn);
    
    const QAbstractItemModel *view_model = view->model();
    
    // Set root to head
    // NOTE: need this to hide head while retaining it in model for drag and drop purposes
    const QModelIndex head_index = view_model->index(0, 0);
    view->setRootIndex(head_index);

    resize_columns();

    const QString target_name = AdInterface::instance()->request_value(target_dn, ATTRIBUTE_NAME);

    QString label_text;
    if (target_name.isEmpty()) {
        label_text = "";
    } else {
        const QModelIndex view_head = view_model->index(0, 0);
        const int object_count = view_model->rowCount(view_head);

        const QString objects_string = tr("%n object(s)", "", object_count);
        label_text = QString("%1: %2").arg(target_name, objects_string);
    }
    label->setText(label_text);
}

void ContentsWidget::resize_columns() {
    const int view_width = view->width();
    const int name_width = (int) (view_width * 0.4);
    const int category_width = (int) (view_width * 0.15);

    view->setColumnWidth(column_index(ATTRIBUTE_NAME), name_width);
    view->setColumnWidth(column_index(ATTRIBUTE_OBJECT_CATEGORY), category_width);
}

void ContentsWidget::showEvent(QShowEvent *event) {
    resize_columns();
}

ContentsModel::ContentsModel(QObject *parent)
: ObjectModel(columns.count(), column_index(ATTRIBUTE_DISTINGUISHED_NAME), parent)
{
    QList<QString> labels;
    for (const QString attribute : columns) {
        const QString attribute_name = get_attribute_display_name(attribute, CLASS_DEFAULT);

        labels.append(attribute_name);
    }

    setHorizontalHeaderLabels(labels);
}

void ContentsModel::change_target(const QString &target_dn) {
    removeRows(0, rowCount());

    if (target_dn == "") {
        return;
    }

    // Load head
    QStandardItem *root = invisibleRootItem();
    const AdObject head_object = AdInterface::instance()->request_all(target_dn);
    make_row(root, head_object);
    QStandardItem *head = item(0, 0);

    // NOTE: get object class as well to get icon
    QList<QString> search_attributes = columns;
    search_attributes.append(ATTRIBUTE_OBJECT_CLASS);

    const QHash<QString, AdObject> search_results = AdInterface::instance()->search("", search_attributes, SearchScope_Children, target_dn);

    // Load children
    for (auto child_dn : search_results.keys()) {
        if (search_results.contains(child_dn)) {
            const AdObject object  = search_results[child_dn];
            make_row(head, object);
        }
    }
}

void ContentsModel::make_row(QStandardItem *parent, const AdObject &object) {
    const QList<QStandardItem *> row = make_item_row(columns.count());

    for (int i = 0; i < columns.count(); i++) {
        const QString attribute = columns[i];
        
        if (!object.contains(attribute)) {
            continue;
        }
        const QByteArray value = object.get_value(attribute);

        const QString value_display =
        [attribute, value]() {
            QString out = attribute_get_display_value(attribute, value);

            // NOTE: category is given as raw DN and contains '-' where it should have spaces, so convert it
            if (attribute == ATTRIBUTE_OBJECT_CATEGORY) {
                out = extract_name_from_dn(out);
                out = out.replace('-', ' ');
            }

            return out;
        }();

        row[i]->setText(value_display);
    }

    const QIcon icon = object.get_icon();
    row[0]->setIcon(icon);

    parent->appendRow(row);
}
