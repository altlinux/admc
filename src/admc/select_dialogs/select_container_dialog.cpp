/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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
#include "ui_select_container_dialog.h"

#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "utils.h"
#include "managers/icon_manager.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

QStandardItem *make_container_node(const AdObject &object);

SelectContainerDialog::SelectContainerDialog(AdInterface &ad, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::SelectContainerDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    ui->view->sortByColumn(0, Qt::AscendingOrder);

    model = new QStandardItemModel(this);

    proxy_model = new QSortFilterProxyModel(this);
    proxy_model->setSourceModel(model);
    proxy_model->setSortCaseSensitivity(Qt::CaseInsensitive);

    ui->view->setModel(proxy_model);

    // Hide all columns except name column
    QHeaderView *header = ui->view->header();
    for (int i = 0; i < header->count(); i++) {
        header->setSectionHidden(i, true);
    }
    header->setSectionHidden(0, false);

    QPushButton *ok_button = ui->button_box->button(QDialogButtonBox::Ok);
    enable_widget_on_selection(ok_button, ui->view);

    // Load head object
    const QString head_dn = g_adconfig->domain_dn();
    const AdObject head_object = ad.search_object(head_dn);
    QStandardItem *item = make_container_node(head_object);
    model->appendRow(item);

    // NOTE: geometry is shared with the subclass
    // MoveObjectDialog but that is intended.
    settings_setup_dialog_geometry(SETTING_select_container_dialog_geometry, this);

    connect(
        ui->view, &QTreeView::expanded,
        this, &SelectContainerDialog::on_item_expanded);
}

SelectContainerDialog::~SelectContainerDialog() {
    delete ui;
}

QString SelectContainerDialog::get_selected() const {
    const QModelIndex selected_index = ui->view->selectionModel()->currentIndex();
    const QString dn = selected_index.data(ContainerRole_DN).toString();

    return dn;
}

void SelectContainerDialog::fetch_node(const QModelIndex &proxy_index) {
    const QModelIndex index = proxy_model->mapToSource(proxy_index);

    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    show_busy_indicator();

    model->removeRows(0, model->rowCount(index), index);

    const QString base = index.data(ContainerRole_DN).toString();
    const SearchScope scope = SearchScope_Children;

    const QString filter = [=]() {
        QString out;

        out = is_container_filter();
        out = advanced_features_filter(out);

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

void SelectContainerDialog::on_item_expanded(const QModelIndex &index) {
    const bool fetched = index.data(ContainerRole_Fetched).toBool();
    if (!fetched) {
        fetch_node(index);
    }
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

    const QIcon icon = g_icon_manager->get_object_icon(object);
    item->setIcon(icon);

    return item;
}
