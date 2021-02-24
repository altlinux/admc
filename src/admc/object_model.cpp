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
#include "ad_interface.h"
#include "ad_object.h"
#include "ad_config.h"
#include "ad_utils.h"
#include "filter.h"
#include "settings.h"
#include "utils.h"
#include "attribute_display.h"
#include "status.h"

#include <QString>

void load_object_row(const QList<QStandardItem *> row, const AdObject &object) {
    // Load attribute columns
    for (int i = 0; i < ADCONFIG()->get_columns().count(); i++) {
        const QString attribute = ADCONFIG()->get_columns()[i];

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
                    return ADCONFIG()->get_class_display_name(object_class);
                }
            } else {
                const QByteArray value = object.get_value(attribute);
                return attribute_display_value(attribute, value);
            }
        }();

        row[i]->setText(display_value);
    }

    const QIcon icon = object.get_icon();
    row[0]->setIcon(icon);

    load_object_item_data(row[0], object);
}

void load_object_item_data(QStandardItem *item, const AdObject &object) {
    item->setData(object.get_dn(), ObjectRole_DN);
    item->setData(QVariant::fromValue(object), ObjectRole_AdObject);
}

QList<QString> object_model_header_labels() {
    QList<QString> out;

    for (const QString &attribute : ADCONFIG()->get_columns()) {
        const QString attribute_display_name = ADCONFIG()->get_column_display_name(attribute);

        out.append(attribute_display_name);
    }
    
    return out;
}
