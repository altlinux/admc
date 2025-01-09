/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "tabs/attributes_tab_proxy.h"

#include "adldap.h"
#include "attribute_dialogs/bool_attribute_dialog.h"
#include "attribute_dialogs/datetime_attribute_dialog.h"
#include "attribute_dialogs/list_attribute_dialog.h"
#include "attribute_dialogs/octet_attribute_dialog.h"
#include "attribute_dialogs/string_attribute_dialog.h"
#include "globals.h"
#include "settings.h"
#include "tabs/attributes_tab.h"
#include "tabs/attributes_tab_filter_menu.h"
#include "utils.h"

#include <QAction>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QStandardItemModel>

QString attribute_type_display_string(const AttributeType type);

AttributesTabProxy::AttributesTabProxy(AttributesTabFilterMenu *filter_menu_arg, QObject *parent)
: QSortFilterProxyModel(parent) {
    filter_menu = filter_menu_arg;
}

void AttributesTabProxy::load(const AdObject &object) {
    const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);

    const QList<QString> mandatory_attributes_list = g_adconfig->get_mandatory_attributes(object_classes);
    mandatory_attributes = QSet<QString>(mandatory_attributes_list.begin(), mandatory_attributes_list.end());

    const QList<QString> optional_attributes_list = g_adconfig->get_optional_attributes(object_classes);
    optional_attributes = QSet<QString>(optional_attributes_list.begin(), optional_attributes_list.end());

    const QList<QString> attributes_list = object.attributes();
    set_attributes = QSet<QString>(attributes_list.begin(), attributes_list.end());
}

void AttributesTabProxy::update_set_attributes(QSet<QString> attributes) {
    set_attributes += attributes;
}

bool AttributesTabProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    auto source = sourceModel();
    const QString attribute = source->index(source_row, AttributesColumn_Name, source_parent).data().toString();

    const bool system_only = g_adconfig->get_attribute_is_system_only(attribute);
    const bool unset = !set_attributes.contains(attribute);
    const bool mandatory = mandatory_attributes.contains(attribute);
    const bool optional = optional_attributes.contains(attribute);

    if (!filter_menu->filter_is_enabled(AttributeFilter_Unset) && unset) {
        return false;
    }

    if (!filter_menu->filter_is_enabled(AttributeFilter_Mandatory) && mandatory) {
        return false;
    }

    if (!filter_menu->filter_is_enabled(AttributeFilter_Optional) && optional) {
        return false;
    }

    if (filter_menu->filter_is_enabled(AttributeFilter_ReadOnly) && system_only) {
        const bool constructed = g_adconfig->get_attribute_is_constructed(attribute);
        const bool backlink = g_adconfig->get_attribute_is_backlink(attribute);

        if (!filter_menu->filter_is_enabled(AttributeFilter_SystemOnly) && !constructed && !backlink) {
            return false;
        }

        if (!filter_menu->filter_is_enabled(AttributeFilter_Constructed) && constructed) {
            return false;
        }

        if (!filter_menu->filter_is_enabled(AttributeFilter_Backlink) && backlink) {
            return false;
        }
    }

    if (!filter_menu->filter_is_enabled(AttributeFilter_ReadOnly) && system_only) {
        return false;
    }

    return true;
}
