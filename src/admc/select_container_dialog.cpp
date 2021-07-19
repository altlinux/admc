/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

QStandardItem *make_container_node(const AdObject &object);

SelectContainerDialog::SelectContainerDialog(QWidget *parent)
: QDialog(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Select Container"));

    AdInterface ad;
    if (ad_failed(ad)) {
        close();
        return;
    }

    resize(400, 500);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setExpandsOnDoubleClick(true);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);
    view->sortByColumn(g_adconfig->get_column_index(ATTRIBUTE_NAME), Qt::AscendingOrder);
    view->setHeaderHidden(true);

    model = new QStandardItemModel(this);

    proxy_model = new QSortFilterProxyModel(this);
    proxy_model->setSourceModel(model);
    proxy_model->setSortCaseSensitivity(Qt::CaseInsensitive);

    view->setModel(proxy_model);

    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);

    // Hide all columns except name column
    QHeaderView *header = view->header();
    for (int i = 0; i < header->count(); i++) {
        header->setSectionHidden(i, true);
    }
    header->setSectionHidden(g_adconfig->get_column_index(ATTRIBUTE_NAME), false);

    enable_widget_on_selection(ok_button, view);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(view);
    layout->addWidget(button_box);

    // Load head object
    const QString head_dn = g_adconfig->domain_head();
    const AdObject head_object = ad.search_object(head_dn);
    auto item = make_container_node(head_object);
    model->appendRow(item);

    g_status()->display_ad_messages(ad, this);

    // NOTE: geometry is shared with the subclass
    // MoveObjectDialog but that is intended.
    g_settings->setup_dialog_geometry(VariantSetting_SelectContainerDialogGeometry, this);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);

    connect(
        view, &QTreeView::expanded,
        [=](const QModelIndex &index) {
            const bool fetched = index.data(ContainerRole_Fetched).toBool();
            if (!fetched) {
                fetch_node(index);
            }
        });
}

QString SelectContainerDialog::get_selected() const {
    const QModelIndex selected_index = view->selectionModel()->currentIndex();
    const QString dn = selected_index.data(ContainerRole_DN).toString();

    return dn;
}

void SelectContainerDialog::fetch_node(const QModelIndex &proxy_index) {
    const QModelIndex index = proxy_model->mapToSource(proxy_index);

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    show_busy_indicator();

    model->removeRows(0, model->rowCount(index), index);

    const QString base = index.data(ContainerRole_DN).toString();
    const SearchScope scope = SearchScope_Children;

    const QString filter = [=]() {
        QString out;

        out = is_container_filter();

        advanced_features_filter(out);
        dev_mode_filter(out);

        return out;
    }();

    const QList<QString> attributes = QList<QString>();

    QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

    dev_mode_search_results(results, ad, base);

    QStandardItem *parent = model->itemFromIndex(index);
    for (const AdObject &object : results.values()) {
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

    const QIcon icon = get_object_icon(object);
    item->setIcon(icon);

    return item;
}
