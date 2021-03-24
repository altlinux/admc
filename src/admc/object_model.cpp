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

#include "object_model.h"
#include "ad/ad_interface.h"
#include "ad/ad_object.h"
#include "ad/ad_config.h"
#include "globals.h"
#include "ad/ad_utils.h"
#include "ad/ad_filter.h"
#include "settings.h"
#include "utils.h"
#include "ad/ad_display.h"
#include "status.h"

#include <QStandardItemModel>

void load_object_row(const QList<QStandardItem *> row, const AdObject &object) {
    // Load attribute columns
    for (int i = 0; i < adconfig->get_columns().count(); i++) {
        const QString attribute = adconfig->get_columns()[i];

        if (!object.contains(attribute)) {
            continue;
        }

        const QString display_value =
        [attribute, object]() {
            if (attribute == ATTRIBUTE_OBJECT_CLASS) {
                const QString object_class = object.get_string(attribute);

                if (object_class == CLASS_GROUP) {
                    const GroupScope scope = object.get_group_scope(); 
                    const QString scope_string = group_scope_string(scope);

                    const GroupType type = object.get_group_type(); 
                    const QString type_string = group_type_string_adjective(type);

                    return QString("%1 - %2").arg(type_string, scope_string);
                } else {
                    return adconfig->get_class_display_name(object_class);
                }
            } else {
                const QByteArray value = object.get_value(attribute);
                return attribute_display_value(attribute, value);
            }
        }();

        row[i]->setText(display_value);
    }

    load_object_item_data(row[0], object);
}

void load_object_item_data(QStandardItem *item, const AdObject &object) {
    const QIcon icon = object.get_icon();
    item->setIcon(icon);
    
    item->setData(object.get_dn(), ObjectRole_DN);

    const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    item->setData(QVariant(object_classes), ObjectRole_ObjectClasses);

    const bool cannot_move = object.get_system_flag(SystemFlagsBit_CannotMove);
    item->setData(cannot_move, ObjectRole_CannotMove);

    const bool cannot_rename = object.get_system_flag(SystemFlagsBit_CannotRename);
    item->setData(cannot_rename, ObjectRole_CannotRename);

    const bool cannot_delete = object.get_system_flag(SystemFlagsBit_CannotDelete);
    item->setData(cannot_delete, ObjectRole_CannotDelete);

    const bool account_disabled = object.get_account_option(AccountOption_Disabled);
    item->setData(account_disabled, ObjectRole_AccountDisabled);
}

QList<QString> object_model_header_labels() {
    QList<QString> out;

    for (const QString &attribute : adconfig->get_columns()) {
        const QString attribute_display_name = adconfig->get_column_display_name(attribute);

        out.append(attribute_display_name);
    }
    
    return out;
}

QList<int> object_model_default_columns() {
    // By default show first 3 columns: name, class and
    // description
    return {0, 1, 2};
}
