/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2025 Semyon Knyazev
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

#include "site_links_table_edit.h"
#include "ad_interface.h"
#include "core/globals.h"
#include "ad_config.h"
#include "ad_filter.h"
#include "ad_object.h"
#include "core/managers/icon_manager.h"
#include <QTableWidget>


SiteLinksTableEdit::SiteLinksTableEdit(QTableWidget *site_links_table_arg, QObject *parent) :
    AttributeEdit(parent),
    site_links_table(site_links_table_arg) {

}

void SiteLinksTableEdit::load(AdInterface &ad, const AdObject &object) {
    Q_UNUSED(object)

    if (!ad.is_connected()) {
        return;
    }

    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_SITE_LINK);
    auto site_link_objs = ad.search(g_adconfig->sites_container_dn(), SearchScope_All, filter,
                                    {ATTRIBUTE_NAME});

    site_links_table->setRowCount(site_link_objs.size());
    int row = 0;
    for (const QString &dn : site_link_objs.keys()) {
        QTableWidgetItem *chain_name_item = new QTableWidgetItem(g_icon_manager->item_icon(ItemIcon_Site_Link),
                                                                 site_link_objs[dn].get_string(ATTRIBUTE_NAME));
        site_links_table->setItem(row, SiteLinksTableColumn_ChainName, chain_name_item);
        chain_name_item->setData(Qt::UserRole, dn);
        chain_name_item->setFlags(chain_name_item->flags() & ~Qt::ItemIsEditable);

        QTableWidgetItem *transport_item = new QTableWidgetItem();
        transport_item->setFlags(transport_item->flags() & ~Qt::ItemIsEditable);
        if (dn.contains("CN=IP")) {
            transport_item->setText("IP");
        } else if (dn.contains("CN=SMTP")) {
            transport_item->setText("CMTP");
        } else {
            transport_item->setText("Undefined");
        }
        site_links_table->setItem(row, SiteLinksTableColumn_Transport, transport_item);
        ++row;
    }

    site_links_table->resizeColumnsToContents();
    site_links_table->selectRow(0);
}

bool SiteLinksTableEdit::apply(AdInterface &ad, const QString &dn) const {
    if (site_links_table->selectedItems().isEmpty()) {
        return true;
    }

    QTableWidgetItem *selected_item = site_links_table->item(site_links_table->selectedItems().first()->row(),
                                                             SiteLinksTableColumn_ChainName);
    const QString site_link_dn = selected_item->data(Qt::UserRole).toString();
    if (site_link_dn.isEmpty()) {
        return false;
    }

    auto site_link_obj = ad.search_object(site_link_dn, {ATTRIBUTE_SITE_LIST});
    if (site_link_obj.is_empty()) {
        return false;
    }

    QList<QByteArray> values = site_link_obj.get_values(ATTRIBUTE_SITE_LIST);
    // remove non-existent sites (which have been deleted, for example)
    // TODO: Remove this kludge later. Site existence in site link's
    // list shouldn't be checked here.
    for (const QByteArray &site_dn_bytes : values) {
        if (!site_exists(ad, site_dn_bytes)) {
            values.removeAll(site_dn_bytes);
        }
    }

    values.append(dn.toUtf8());

    bool res = ad.attribute_replace_values(site_link_dn, ATTRIBUTE_SITE_LIST, values);
    return res;
}

bool SiteLinksTableEdit::site_exists(AdInterface &ad, const QString &site_dn) const {
    AdObject obj = ad.search_object(site_dn, {ATTRIBUTE_DN});
    return !obj.is_empty();
}

