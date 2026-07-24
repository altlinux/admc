/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2025-2026 BaseALT Ltd.
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

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>

#include <attribute_edits/string_edit.h>
#include "ad_config.h"
#include "ad_defines.h"
#include "ad_filter.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "ad_utils.h"
#include "globals.h"
#include "managers/icon_manager.h"
#include "sites_link_edit.h"
#include "tabs/sites_link_tab/sites_link_common_widget.h"
#include "tabs/sites_link_tab/sites_link_part_widget.h"
#include "tabs/sites_link_tab/sites_link_widget.h"
#include "utils.h"

SitesLinkEdit::SitesLinkEdit(SitesLinkWidget *link_wget_arg, QObject *parent) :
    AttributeEdit(parent),
    sites_link_common_wget(link_wget_arg->common_widget()),
    sites_link_part_wget(link_wget_arg->sites_link_part_widget()),
    type(link_wget_arg->get_type())
{
    is_link_type = (type == SitesLinkType::Link);
    setup_widgets();
}

SitesLinkEdit::SitesLinkEdit(SitesLinkType type_arg,
                             SitesLinkCommonWidget *common_link_wget_arg,
                             QObject *parent) :
    AttributeEdit(parent),
    sites_link_common_wget(common_link_wget_arg),
    sites_link_part_wget(nullptr),
    type(type_arg)
{
    is_link_type = (type == SitesLinkType::Link);
    setup_widgets();
}

void SitesLinkEdit::load(AdInterface &ad, const AdObject &object) {
    if ((! ad.is_connected()) || (! sites_link_common_wget)) {
        return;
    }

    sites_link_common_wget->right_list_wget()->clear();
    sites_link_common_wget->left_list_wget()->clear();

    description_edit->load(ad, object);

    const QStringList linked_dn_list = is_link_type ?
        object.get_strings(ATTRIBUTE_SITE_LIST) :
        object.get_strings(ATTRIBUTE_SITE_LINK_LIST);
    const QIcon item_icon = is_link_type ?
        g_icon_manager->item_icon(ItemIcon_Site) :
        g_icon_manager->item_icon(ItemIcon_Site_Link);

    for (const QString &dn : linked_dn_list) {
        QListWidgetItem *item = new QListWidgetItem(item_icon, dn_get_name(dn));
        item->setData(Qt::UserRole, dn);
        sites_link_common_wget->right_list_wget()->addItem(item);
    }

    const QString search_category =
        is_link_type ? OBJECT_CATEGORY_SITE : OBJECT_CATEGORY_SITE_LINK;
    const QString filter = filter_CONDITION(Condition_Equals,
                                            ATTRIBUTE_OBJECT_CATEGORY,
                                            search_category);
    auto search_res = ad.search(g_adconfig->sites_container_dn(),
                                SearchScope_Children,
                                filter,
                                { ATTRIBUTE_DN });

    for (const QString &dn : search_res.keys()) {
        if (!linked_dn_list.contains(dn)) {
            QListWidgetItem *item =
                new QListWidgetItem(item_icon, dn_get_name(dn));
            item->setData(Qt::UserRole, dn);
            sites_link_common_wget->left_list_wget()->addItem(item);
        }
    }

    if (is_link_type && sites_link_part_wget) {
        int cost = object.get_int(ATTRIBUTE_LINK_COST);
        sites_link_part_wget->cost_spinbox()->setValue(cost);

        int repl_interval = object.get_int(ATTRIBUTE_LINK_REPLICATION_INTERVAL);
        sites_link_part_wget->replicate_spinbox()->setValue(repl_interval);
    }
}

bool SitesLinkEdit::apply(AdInterface &ad, const QString &dn) const {
    if ((! ad.is_connected()) || (! sites_link_common_wget)) {
        return false;
    }

    QHash<QString, QList<QByteArray>> values = {};

    if (! description_edit->apply(ad, dn)) {
        return false;
    }

    const QString link_attr =
        is_link_type ? ATTRIBUTE_SITE_LIST : ATTRIBUTE_SITE_LINK_LIST;
    auto right_list_wget = sites_link_common_wget->right_list_wget();
    const int count = right_list_wget->count();
    for (int i = 0; i < count; ++i) {
        auto linked_dn =
            right_list_wget->item(i)->data(Qt::UserRole).toString();
        if (! linked_dn.isEmpty()) {
            values[link_attr] << linked_dn.toUtf8();
        }
    }

    if (is_link_type && sites_link_part_wget) {
        int cost = sites_link_part_wget->cost_spinbox()->value();
        values[ATTRIBUTE_LINK_COST] << QByteArray::number(cost);

        int repl_interval = sites_link_part_wget->replicate_spinbox()->value();
        values[ATTRIBUTE_LINK_REPLICATION_INTERVAL] <<
            QByteArray::number(repl_interval);
    }

    for (auto attr : values.keys()) {
        if (! ad.attribute_replace_values(dn, attr, values[attr])) {
            return false;
        }
    }

    return true;
}

bool SitesLinkEdit::verify(AdInterface &ad, const QString &dn) const {
    Q_UNUSED(ad);
    Q_UNUSED(dn);

    const int min_linked_objects_count = 2;
    auto right_list_wget = sites_link_common_wget->right_list_wget();
    if (right_list_wget->count() < min_linked_objects_count) {
        const QString warning_text = is_link_type ?
                    tr("Site link object must link at least two sites") :
                    tr("Link bridge object must link at least two site links");
        message_box_warning(sites_link_common_wget, tr("Error"), warning_text);
        return false;
    }

    return true;
}

void SitesLinkEdit::setup_widgets() {
    if (!sites_link_common_wget) {
        return;
    }

    description_edit = new StringEdit(
        sites_link_common_wget->description_line_edit(),
        ATTRIBUTE_DESCRIPTION,
        this);
    connect(description_edit,
            &AttributeEdit::edited,
            this,
            &AttributeEdit::edited);

    sites_link_common_wget->set_lists_labels(type);

    connect(sites_link_common_wget->add_button(),
            &QPushButton::clicked,
            this,
            &AttributeEdit::edited);
    connect(sites_link_common_wget->remove_button(),
            &QPushButton::clicked,
            this,
            &AttributeEdit::edited);

    if (sites_link_part_wget) {
        connect(sites_link_part_wget->replicate_spinbox(),
                &QSpinBox::textChanged,
                this, [this](const QString&) {
            emit AttributeEdit::edited();
        });
        connect(sites_link_part_wget->cost_spinbox(),
                &QSpinBox::textChanged,
                this, [this](const QString&) {
            emit AttributeEdit::edited();
        });
    }
}
