/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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
#include "core/managers/icon_manager.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

QStandardItem *make_container_node(const AdObject &object);

SelectContainerDialog::SelectContainerDialog(AdInterface &ad, QWidget *parent, const QStringList &obj_dn_list)
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

    // TODO: Split that switch/dialog onto corresponding polimorphic dialogs
    // for each selection case
    ParentContainerType container_type = parent_container_type(obj_dn_list);
    switch (container_type) {
    case ParentContainerType_Default:
        setup_default_container_tree(ad);
        break;
    case ParentContainerType_SiteServers:
        setup_site_container_list(ad, obj_dn_list);
        break;
    default:
        setup_undefined_view_state();
    }

    // NOTE: geometry is shared with the subclass
    // MoveObjectDialog but that is intended.
    settings_setup_dialog_geometry(SETTING_select_container_dialog_geometry, this);
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
    const QString filter = advanced_features_filter(is_container_filter());
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

SelectContainerDialog::ParentContainerType SelectContainerDialog::parent_container_type(const QStringList &obj_dn_list) {
    ParentContainerType type = ParentContainerType_Default;

    if (obj_dn_list.isEmpty()) {
        return type;
    }

    if (obj_dn_list.first().contains(g_adconfig->sites_container_dn())) {
        type = ParentContainerType_SiteServers;
    }

    // Check if site child objects are not mixed with "standard" objects like user, computer, OU, etc.
    // According to current console UI it shouldn't happen, but check just in case.
    for (const QString &obj_dn : obj_dn_list) {
        if ((!obj_dn.contains(g_adconfig->sites_container_dn()) && type == ParentContainerType_SiteServers) ||
                (obj_dn.contains(g_adconfig->sites_container_dn()) && type == ParentContainerType_Default)) {
            type = ParentContainerType_Undefined;
            break;
        }
    }

    return type;
}

void SelectContainerDialog::setup_default_container_tree(AdInterface &ad) {
    // Load head object
    const QString head_dn = g_adconfig->domain_dn();
    const AdObject head_object = ad.search_object(head_dn);
    QStandardItem *item = make_container_node(head_object);
    model->appendRow(item);

    connect(
        ui->view, &QTreeView::expanded,
        this, &SelectContainerDialog::on_item_expanded);
}

void SelectContainerDialog::setup_site_container_list(AdInterface &ad, const QStringList &obj_dn_list) {
    if (obj_dn_list.isEmpty()) {
        return;
    }

    // Setup tree view as list
    ui->view->setIndentation(0);
    ui->view->setRootIsDecorated(false);
    ui->view->setItemsExpandable(false);
    ui->view->setHeaderHidden(true);
    ui->view->setUniformRowHeights(true);

    // Get current parent site container to exclude it from the list (Servers container actually)
    const QString parent_dn = dn_get_parent(obj_dn_list.first());

    // Check if the objects have different parent to move from.
    // In theory, this shouldn't happen, but for this exotic case list will contain all sites.
    bool same_parent = true;
    for (const QString &obj_dn : obj_dn_list) {
        if (!obj_dn.contains(parent_dn)) {
            same_parent = false;
        }
    }

    const QString sites_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_SITE);
    auto sites = ad.search(g_adconfig->sites_container_dn(), SearchScope_Children, sites_filter, {ATTRIBUTE_NAME});

    for (const QString &site_dn : sites.keys()) {
        if (parent_dn.contains(site_dn) && same_parent) {
            continue;
        }

        const QString servers_container_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_SERVERS_CONTAINER);
        auto servers_container = ad.search(site_dn, SearchScope_Children, servers_container_filter, {ATTRIBUTE_DN});
        if (servers_container.isEmpty()) {
            continue;
        }

        // Site's Servers container DN will be written as site DN data and will be returned
        // with get_selected(). It allows to move server class objects between sites.
        const QString servers_container_dn = servers_container.keys().first();
        const QString site_name = sites[site_dn].get_string(ATTRIBUTE_NAME);
        QStandardItem *site_item = new QStandardItem(g_icon_manager->item_icon(ItemIcon_Site), site_name);
        site_item->setData(servers_container_dn, ContainerRole_DN);

        model->appendRow(site_item);
    }
}

void SelectContainerDialog::setup_undefined_view_state() {
    QStandardItem *item = new QStandardItem(tr("Failed to define suitable containers"));
    model->appendRow(item);
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

    const QIcon icon = g_icon_manager->object_icon(object);
    item->setIcon(icon);

    return item;
}
