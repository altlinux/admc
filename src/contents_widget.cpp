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
#include "dn_column_proxy.h"
#include "utils.h"

#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>

enum ContentsColumn {
    ContentsColumn_Name,
    ContentsColumn_Category,
    ContentsColumn_Description,
    ContentsColumn_DN,
    ContentsColumn_COUNT,
};

ContentsWidget::ContentsWidget(ContainersWidget *containers_widget, ObjectContextMenu *object_context_menu, QWidget *parent)
: QWidget(parent)
{   
    model = new ContentsModel(this);
    const auto advanced_view_proxy = new AdvancedViewProxy(ContentsColumn_DN, this);
    const auto dn_column_proxy = new DnColumnProxy(ContentsColumn_DN, this);

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
    object_context_menu->connect_view(view, ContentsColumn_DN);

    setup_model_chain(view, model, {advanced_view_proxy, dn_column_proxy});

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
        AD(), &AdInterface::modified,
        this, &ContentsWidget::on_ad_modified);

    connect(
        view, &QAbstractItemView::clicked,
        [this] (const QModelIndex &index) {
            const QString dn = get_dn_from_index(index, ContentsColumn_DN);

            emit clicked_dn(dn);
        });
}

void ContentsWidget::on_containers_selected_changed(const QString &dn) {
    model->change_target(dn);
    set_root_to_head(view);
    resize_columns();

    const QString target_name = AD()->attribute_get(dn, "name");

    QString label_text;
    if (target_name.isEmpty()) {
        label_text = "";
    } else {
        const QAbstractItemModel *view_model = view->model();
        const QModelIndex view_head = view_model->index(0, 0);
        const int object_count = view_model->rowCount(view_head);

        label_text = QString("%1: %2 objects").arg(target_name).arg(object_count);
    }
    label->setText(label_text);
}

void ContentsWidget::on_ad_modified() {
    model->change_target(model->target_dn);
    set_root_to_head(view);
}

void ContentsWidget::resize_columns() {
    const int view_width = view->width();
    const int name_width = (int) (view_width * 0.4);
    const int category_width = (int) (view_width * 0.15);
    view->setColumnWidth(ContentsColumn_Name, name_width);
    view->setColumnWidth(ContentsColumn_Category, category_width);
}

void ContentsWidget::showEvent(QShowEvent *event) {
    resize_columns();
}

ContentsModel::ContentsModel(QObject *parent)
: ObjectModel(ContentsColumn_COUNT, ContentsColumn_DN, parent)
{
    setHorizontalHeaderItem(ContentsColumn_Name, new QStandardItem("Name"));
    setHorizontalHeaderItem(ContentsColumn_Category, new QStandardItem("Category"));
    setHorizontalHeaderItem(ContentsColumn_Description, new QStandardItem("Description"));
    setHorizontalHeaderItem(ContentsColumn_DN, new QStandardItem("DN"));
}

void ContentsModel::change_target(const QString &dn) {
    removeRows(0, rowCount());

    if (dn == "") {
        return;
    }

    // Load head
    target_dn = dn;
    QStandardItem *root = invisibleRootItem();    
    make_new_row(root, target_dn);
    QStandardItem *head = item(0, 0);

    // Load children
    QList<QString> children = AD()->list(dn);
    for (auto child_dn : children) {
        make_new_row(head, child_dn);
    }
}

void ContentsModel::load_row(QList<QStandardItem *> row, const QString &dn) {
    QString name = AD()->attribute_get(dn, "name");

    // NOTE: this is given as raw DN and contains '-' where it should
    // have spaces, so convert it
    QString category = AD()->attribute_get(dn, "objectCategory");
    category = extract_name_from_dn(category);
    category = category.replace('-', ' ');

    QString description = AD()->attribute_get(dn, "description");

    row[ContentsColumn_Name]->setText(name);
    row[ContentsColumn_Category]->setText(category);
    row[ContentsColumn_Description]->setText(description);
    row[ContentsColumn_DN]->setText(dn);

    QIcon icon = get_object_icon(dn);
    row[0]->setIcon(icon);
}

void ContentsModel::make_new_row(QStandardItem *parent, const QString &dn) {
    auto row = QList<QStandardItem *>();
    for (int i = 0; i < ContentsColumn_COUNT; i++) {
        row.push_back(new QStandardItem());
    }

    load_row(row, dn);

    parent->appendRow(row);
}
