/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#include "domain_info_results_widget.h"
#include "ui_domain_info_results_widget.h"
#include "console_widget/console_widget.h"
#include "ad_interface.h"
#include "settings.h"
#include "ad_defines.h"
#include "console_impls/object_impl.h"
#include "console_impls/item_type.h"
#include "globals.h"
#include "ad_config.h"
#include "status.h"
#include "fsmo/fsmo_utils.h"
#include "managers/icon_manager.h"
#include "utils.h"

#include <QStandardItemModel>
#include <QPushButton>

DomainInfoResultsWidget::DomainInfoResultsWidget(ConsoleWidget *console_arg) :
    QWidget(console_arg), ui(new Ui::DomainInfoResultsWidget), console(console_arg) {
    ui->setupUi(this);

    model = new QStandardItemModel(this);
    ui->tree->setModel(model);
    ui->tree->setHeaderHidden(true);

    connect(console, &ConsoleWidget::fsmo_master_changed, this, &DomainInfoResultsWidget::update_fsmo_roles);
    update();
}

DomainInfoResultsWidget::~DomainInfoResultsWidget() {
    delete ui;
}

void DomainInfoResultsWidget::update() {
    update_defaults();

    DomainInfo_SearchResults results = search_results();
    populate_widgets(results);
}

void DomainInfoResultsWidget::update_fsmo_roles(const QString &new_master_dn, const QString &fsmo_role_string) {
    QList<QStandardItem*> fsmo_role_item_list = model->findItems(fsmo_role_string, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
    if (fsmo_role_item_list.isEmpty()) {
        return;
    }

    const QModelIndex start_index = model->index(0, 0, QModelIndex());
    QModelIndexList new_master_index_list = model->match(start_index, DomainInfoTreeItemRole_DN,
                                                         new_master_dn, 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
    if (new_master_index_list.isEmpty()) {
        return;
    }

    QStandardItem *fsmo_role_item = fsmo_role_item_list[0];
    QModelIndex new_master_index = new_master_index_list[0];
    QModelIndex roles_container_index = model->match(new_master_index, DomainInfoTreeItemRole_ItemType, DomainInfoTreeItemType_FSMO_Container,
                                                     1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive))[0];
    model->itemFromIndex(roles_container_index)->appendRow(
                fsmo_role_item->parent()->takeRow(fsmo_role_item->row()));
}

void DomainInfoResultsWidget::update_defaults() {
    model->clear();

    const QList<QLabel*> labels {
        ui->sites_count_value,
        ui->dc_count_value,
        ui->domain_functionality_value,
        ui->forest_functionality_value,
        ui->domain_schema_value,
        ui->dc_version_value
    };
    for (auto label : labels) {
        set_label_failed(label, false);
    }
}

QList<QStandardItem *> DomainInfoResultsWidget::get_tree_items(AdInterface &ad) {
    const QString sites_container_dn = "CN=Sites,CN=Configuration," + g_adconfig->root_domain_dn();
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_SITE);
    const QHash<QString, AdObject> site_objects = ad.search(sites_container_dn, SearchScope_Children,
                                                            filter, {ATTRIBUTE_DN, ATTRIBUTE_NAME/*, ATTRIBUTE_GPLINK*/});

    QList<QStandardItem *> site_items;
    if (site_objects.isEmpty()) {
        g_status->add_message("Failed to find domain sites.", StatusType_Error);
        return site_items;
    }

    for (const AdObject &site_object : site_objects.values()) {
        QStandardItem *site_item = add_tree_item(site_object.get_string(ATTRIBUTE_NAME), g_icon_manager->get_icon_for_type(ItemIconType_Site_Clean),
                                                 DomainInfoTreeItemType_Site, nullptr);
        site_item->setData(site_object.get_dn(), DomainInfoTreeItemRole_DN);

        add_host_items(site_item, site_object, ad);

        site_items.append(site_item);
    }

    return site_items;
}

DomainInfo_SearchResults DomainInfoResultsWidget::search_results() {
    DomainInfo_SearchResults results;

    AdInterface ad;
    if (!ad.is_connected()) {
        return results;
    }

    results.tree_items = get_tree_items(ad);

    const QStringList root_dse_attributes = {
        ATTRIBUTE_DOMAIN_FUNCTIONALITY_LEVEL,
        ATTRIBUTE_FOREST_FUNCTIONALITY_LEVEL,
        ATTRIBUTE_SCHEMA_NAMING_CONTEXT,
        ATTRIBUTE_DNS_HOST_NAME,
        ATTRIBUTE_SERVER_NAME
    };

    const AdObject rootDSE = ad.search_object("", root_dse_attributes);

    const QString server_name = rootDSE.get_string(ATTRIBUTE_SERVER_NAME);
    const AdObject server_object = ad.search_object(server_name, {ATTRIBUTE_SERVER_REFERENCE});
    const AdObject host = ad.search_object(server_object.get_string(ATTRIBUTE_SERVER_REFERENCE), {ATTRIBUTE_OS, ATTRIBUTE_OS_VERSION});
    const QString dc_version = host.get_string(ATTRIBUTE_OS).isEmpty() ? QString() : host.get_string(ATTRIBUTE_OS) +
                                                                                    QString(" (%1)").arg(host.get_string(ATTRIBUTE_OS_VERSION));
    results.domain_controller_version = dc_version;

    const int forest_level = rootDSE.get_int(ATTRIBUTE_FOREST_FUNCTIONALITY_LEVEL);
    const QString forest_level_string = QString::number(forest_level) + " " + functionality_level_to_string(forest_level);
    results.forest_functional_level = forest_level_string;

    const int domain_level = rootDSE.get_int(ATTRIBUTE_DOMAIN_FUNCTIONALITY_LEVEL);
    const QString domain_level_string = QString::number(domain_level) + " " + functionality_level_to_string(domain_level);
    results.domain_functional_level = domain_level_string;

    const QString schema_dn = rootDSE.get_string(ATTRIBUTE_SCHEMA_NAMING_CONTEXT);
    const AdObject schema_object = ad.search_object(schema_dn, {ATTRIBUTE_OBJECT_VERSION});
    const int domain_schema_version = schema_object.get_int(ATTRIBUTE_OBJECT_VERSION);
    const QString domain_schema_version_string = QString::number(domain_schema_version) + " " + schema_version_to_string(domain_schema_version);
    results.domain_schema_version = domain_schema_version_string;

    return results;
}

void DomainInfoResultsWidget::populate_widgets(DomainInfo_SearchResults results) {
    for (auto item : results.tree_items) {
        model->appendRow(item);
    }
    model->sort(0);
    ui->tree->expandToDepth(1);

    // Set dc and sites count to label text
    // Search is performed in the tree to avoid excessive ldap queries
    const QHash<int, QLabel*> type_label_hash {
        {DomainInfoTreeItemType_Site, ui->sites_count_value},
        {DomainInfoTreeItemType_Host, ui->dc_count_value}
    };

    for (int type : type_label_hash.keys()) {
        const QModelIndex start_index = model->index(0, 0, QModelIndex());
        int count = model->match(start_index, DomainInfoTreeItemRole_ItemType, type,
                             -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive)).count();
        if (!count) {
            set_label_failed(type_label_hash[type], true);
        }
        type_label_hash[type]->setText(QString::number(count));
    }

    // Populate labels. Widgets are keys because levels can match
    const QHash<QLabel*, QString> label_results_hash {
        {ui->domain_functionality_value, results.domain_functional_level},
        {ui->forest_functionality_value, results.forest_functional_level},
        {ui->domain_schema_value, results.domain_schema_version},
        {ui->dc_version_value, results.domain_controller_version}
    };
    for (QLabel *label : label_results_hash.keys()) {
        if (label_results_hash[label].isEmpty()) {
            set_label_failed(label, true);
        }
        else {
            label->setText(label_results_hash[label]);
        }
    }
}

void DomainInfoResultsWidget::add_host_items(QStandardItem *site_item, const AdObject &site_object, AdInterface &ad) {
    const QString servers_container_dn = "CN=Servers," + site_object.get_dn();
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_SERVER);
    const QHash<QString, AdObject> host_objects = ad.search(servers_container_dn, SearchScope_Children,
                                                            filter, {ATTRIBUTE_DN, ATTRIBUTE_NAME, ATTRIBUTE_DNS_HOST_NAME});

    QHash<QString, QStringList> host_fsmo_map;
    for (int role = 0; role < FSMORole_COUNT; ++role) {
        const QString host = current_master_for_role(ad, FSMORole(role));
        host_fsmo_map[host].append(string_fsmo_role(FSMORole(role)));
    }
    QString host;
    for (const AdObject &host_object : host_objects.values()) {
        QStandardItem *host_item = add_tree_item(host_object.get_string(ATTRIBUTE_DNS_HOST_NAME), g_icon_manager->get_icon_for_type(ItemIconType_Domain_Clean),
                                                 DomainInfoTreeItemType_Host, site_item);
        host_item->setData(host_object.get_dn(), DomainInfoTreeItemRole_DN);
        host = host_object.get_string(ATTRIBUTE_DNS_HOST_NAME);

        QStandardItem *fsmo_roles_container_item = add_tree_item(tr("FSMO roles"), g_icon_manager->get_object_icon(ADMC_CATEGORY_FSMO_ROLE_CONTAINER),
                                                                 DomainInfoTreeItemType_FSMO_Container, host_item);

        if (host_fsmo_map.keys().contains(host_item->text())) {
            for (const QString &fsmo_role : host_fsmo_map[host_item->text()]) {
                add_tree_item(fsmo_role, g_icon_manager->get_object_icon(ADMC_CATEGORY_FSMO_ROLE),
                              DomainInfoTreeItemType_FSMO_Role,fsmo_roles_container_item);
            }
        }
    }
}

void DomainInfoResultsWidget::set_label_failed(QLabel *label, bool failed)
{
    if (failed) {
        label->setStyleSheet("color: red");
        label->setText(tr("Undefined"));
    }
    else {
        label->setStyleSheet("");
        label->setText("");
    }
}

QStandardItem* DomainInfoResultsWidget::add_tree_item(const QString &text, const QIcon &icon, DomainInfoTreeItemType type, QStandardItem *parent_item) {
    QStandardItem *item = new QStandardItem(icon, text);
    item->setEditable(false);
    item->setData(type, DomainInfoTreeItemRole_ItemType);
    if (parent_item) {
        parent_item->appendRow(item);
    }
    return item;
}

QString DomainInfoResultsWidget::schema_version_to_string(int version) {
    switch (version) {
        case DomainSchemaVersion_WindowsServer2008R2:
            return "(Windows Server 2008R2)";

        case DomainSchemaVersion_WindowsServer2012:
            return "(Windows Server 2012)";

        case DomainSchemaVersion_WindowsServer2012R2:
            return "(Windows Server 2012R2)";

        case DomainSchemaVersion_WindowsServer2016:
            return "(Windows Server 2016)";

        case DomainSchemaVersion_WindowsServer2019:
            return "(Windows Server 2019/2022)";

        default:
            return QString();
    }
}

QString DomainInfoResultsWidget::functionality_level_to_string(int level) {
    switch (level) {
        case 2:
            return "(Windows Server 2003)";
        case 3:
            return "(Windows Server 2008)";
        case 4:
            return schema_version_to_string(DomainSchemaVersion_WindowsServer2008R2);
        case 5:
            return schema_version_to_string(DomainSchemaVersion_WindowsServer2012);
        case 6:
            return schema_version_to_string(DomainSchemaVersion_WindowsServer2012R2);
        case 7:
            return schema_version_to_string(DomainSchemaVersion_WindowsServer2016);
        default:
            return QString();
    }
}
