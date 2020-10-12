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
#include "object_context_menu.h"
#include "details_dialog.h"
#include "settings.h"
#include "utils.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "settings.h"
#include "filter.h"
#include "attribute_display.h"

#include <QTreeView>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QGridLayout>

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
            ATTRIBUTE_OBJECT_CLASS,
            ATTRIBUTE_DESCRIPTION
        };
        // NOTE: dn is not one of ADUC's columns, but adding it here for convenience
        columns.append(ATTRIBUTE_DISTINGUISHED_NAME);
        const QList<QString> extra_columns =ADCONFIG()->get_extra_columns();
        columns.append(extra_columns);
    }

    model = new ContentsModel(this);

    auto proxy_name = new QSortFilterProxyModel(this);
    proxy_name->setFilterKeyColumn(column_index(ATTRIBUTE_NAME));
    proxy_name->setRecursiveFilteringEnabled(true);

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
    ObjectContextMenu::connect_view(view, column_index(ATTRIBUTE_DISTINGUISHED_NAME));

    proxy_name->setSourceModel(model);
    view->setModel(proxy_name);

    setup_column_toggle_menu(view, model, 
    {
        column_index(ATTRIBUTE_NAME),
        column_index(ATTRIBUTE_OBJECT_CLASS),
        column_index(ATTRIBUTE_DESCRIPTION)
    });

    label = new QLabel(this);

    const auto filter_name_label = new QLabel(tr("Filter: "), this);
    auto filter_name_edit = new QLineEdit(this);

    const auto layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(label, 0, 0);
    layout->setColumnStretch(1, 1);
    layout->addWidget(filter_name_label, 0, 2, Qt::AlignRight);
    layout->addWidget(filter_name_edit, 0, 3);
    layout->addWidget(view, 1, 0, 2, 4);

    connect(
        containers_widget, &ContainersWidget::selected_changed,
        this, &ContentsWidget::on_containers_selected_changed);
    connect(
        AD(), &AdInterface::modified,
        this, &ContentsWidget::on_ad_modified);
    connect(
        view, &QAbstractItemView::clicked,
        this, &ContentsWidget::on_view_clicked);

    const BoolSettingSignal *advanced_view_setting = SETTINGS()->get_bool_signal(BoolSetting_AdvancedView);
    connect(
        advanced_view_setting, &BoolSettingSignal::changed,
        [this]() {
            change_target(target_dn);
        });

    connect(
        filter_name_edit, &QLineEdit::textChanged,
        [proxy_name](const QString &text) {
            proxy_name->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive, QRegExp::FixedString));
        });
    filter_name_edit->setText("");
}

void ContentsWidget::on_containers_selected_changed(const QString &dn) {
    change_target(dn);
}

void ContentsWidget::on_ad_modified() {
    change_target(target_dn);
}

void ContentsWidget::on_view_clicked(const QModelIndex &index) {
    const bool details_from_contents = SETTINGS()->get_bool(BoolSetting_DetailsFromContents);

    if (details_from_contents) {
        const QString dn = get_dn_from_index(index, column_index(ATTRIBUTE_DISTINGUISHED_NAME));
        DetailsDialog::open_for_target(dn);
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

    const QString target_name = AD()->request_value(target_dn, ATTRIBUTE_NAME);

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
    view->setColumnWidth(column_index(ATTRIBUTE_OBJECT_CLASS), category_width);
}

void ContentsWidget::showEvent(QShowEvent *event) {
    resize_columns();
}

ContentsModel::ContentsModel(QObject *parent)
: ObjectModel(columns.count(), column_index(ATTRIBUTE_DISTINGUISHED_NAME), parent)
{
    QList<QString> labels;
    for (const QString attribute : columns) {
        const QString attribute_name = ADCONFIG()->get_attribute_display_name(attribute, CLASS_DEFAULT);

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
    const AdObject head_object = AD()->request_all(target_dn);
    make_row(root, head_object);
    QStandardItem *head = item(0, 0);

    // NOTE: get object class as well to get icon
    QList<QString> search_attributes = columns;
    search_attributes.append(ATTRIBUTE_OBJECT_CLASS);

    const QString filter = current_advanced_view_filter();

    const QHash<QString, AdObject> search_results = AD()->search(filter, search_attributes, SearchScope_Children, target_dn);

    // Load children
    for (auto child_dn : search_results.keys()) {
        if (search_results.contains(child_dn)) {
            const AdObject object  = search_results[child_dn];
            make_row(head, object);
        }
    }

    sort(column_index(ATTRIBUTE_NAME));
}

void ContentsModel::make_row(QStandardItem *parent, const AdObject &object) {
    const QList<QStandardItem *> row = make_item_row(columns.count());

    for (int i = 0; i < columns.count(); i++) {
        const QString attribute = columns[i];
        
        if (!object.contains(attribute)) {
            continue;
        }

        const QString display_value =
        [attribute, object]() {
            if (attribute == ATTRIBUTE_OBJECT_CLASS) {
                // NOTE: last class in the list is the furthest by inheritance so display that one
                const QString object_class = object.get_class();
                
                return ADCONFIG()->get_class_display_name(object_class);
            } else {
                const QByteArray value = object.get_bytes(attribute);
                return attribute_display_value(attribute, value);
            }
        }();

        row[i]->setText(display_value);
    }

    const QIcon icon = object.get_icon();
    row[0]->setIcon(icon);

    parent->appendRow(row);
}
