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

#include "select_container_dialog.h"

#include "utils.h"
#include "ad/ad_interface.h"
#include "ad/ad_config.h"
#include "ad/ad_defines.h"
#include "ad/ad_utils.h"
#include "ad/ad_filter.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHeaderView>
#include <QStandardItemModel>

enum ContainerRole {
    ContainerRole_DN = Qt::UserRole + 1,
    ContainerRole_Fetched = Qt::UserRole + 2,
};

QStandardItem *make_container_node(const AdObject &object);

SelectContainerDialog::SelectContainerDialog(QWidget *parent)
: QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Select a container"));
    
    AdInterface ad;
    if (ad_failed(ad)) {
        close();
        return;
    }
    
    resize(400, 500);

    model = new QStandardItemModel(this);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setExpandsOnDoubleClick(true);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);
    view->sortByColumn(ADCONFIG()->get_column_index(ATTRIBUTE_NAME), Qt::AscendingOrder);
    view->setHeaderHidden(true);

    view->setModel(model);

    auto buttonbox = new QDialogButtonBox();
    auto ok_button = buttonbox->addButton(QDialogButtonBox::Ok);
    buttonbox->addButton(QDialogButtonBox::Cancel);

    // Hide all columns except name column
    QHeaderView *header = view->header();
    for (int i = 0; i < header->count(); i++) {
        header->setSectionHidden(i, true);
    }
    header->setSectionHidden(ADCONFIG()->get_column_index(ATTRIBUTE_NAME), false);

    connect(
        buttonbox, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        buttonbox, &QDialogButtonBox::rejected,
        this, &QDialog::reject);

    connect(
        view, &QTreeView::expanded,
        [=](const QModelIndex &index) {
            const bool fetched = index.data(ContainerRole_Fetched).toBool();
            if (!fetched) {
                fetch_node(index);
            }
        });

    enable_widget_on_selection(ok_button, view);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(view);
    layout->addWidget(buttonbox);

    // Load head object
    const QString head_dn = ADCONFIG()->domain_head();
    const AdObject head_object = ad.search_object(head_dn);
    auto item = make_container_node(head_object);
    model->appendRow(item);
}

QString SelectContainerDialog::get_selected() const {
    const QModelIndex selected_index = view->selectionModel()->currentIndex();
    const QString dn = selected_index.data(ContainerRole_DN).toString();

    return dn;
}

void SelectContainerDialog::fetch_node(const QModelIndex &index) {
    // TODO: handle error
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    show_busy_indicator();

    model->removeRows(0, model->rowCount(index), index);

    const QString filter =
    [=]() {
        const QString is_container = is_container_filter();

        return add_advanced_view_filter(is_container);
    }();

    const QList<QString> search_attributes;

    const QString dn = index.data(ContainerRole_DN).toString();

    const QHash<QString, AdObject> search_results = ad.search(filter, search_attributes, SearchScope_Children, dn);

    QStandardItem *parent = model->itemFromIndex(index);
    for (const AdObject &object : search_results.values()) {
        auto item = make_container_node(object);
        parent->appendRow(item);
    }

    parent->setData(true, ContainerRole_Fetched);

    hide_busy_indicator();
}

QStandardItem *make_container_node(const AdObject &object) {
    auto item = new QStandardItem();
    item->setData(false, ContainerRole_Fetched);

    // NOTE: add fake child to new items, so that the child indicator is shown while they are childless until they are fetched
    item->appendRow(new QStandardItem());

    const QString dn = object.get_dn();
    item->setData(dn, ContainerRole_DN);

    const QString name = dn_get_name(dn);
    item->setText(name);

    const QIcon icon = object.get_icon();
    item->setIcon(icon);

    return item;
}
