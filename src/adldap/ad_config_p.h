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

#ifndef AD_CONFIG_P_H
#define AD_CONFIG_P_H

#include "ad_object.h"

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QString>

// NOTE: name strings to reduce confusion
typedef QString ObjectClass;
typedef QString Attribute;

class AdConfigPrivate {

public:
    AdConfigPrivate();

    QString domain;
    QString domain_dn;
    QString configuration_dn;
    QString schema_dn;

    QList<ObjectClass> filter_containers;

    QList<Attribute> columns;
    QHash<Attribute, QString> column_display_names;

    QHash<ObjectClass, QString> class_display_names;
    QHash<ObjectClass, QList<Attribute>> find_attributes;
    QHash<ObjectClass, QHash<Attribute, QString>> attribute_display_names;

    QHash<Attribute, AdObject> attribute_schemas;
    QHash<ObjectClass, AdObject> class_schemas;

    QList<ObjectClass> add_auxiliary_classes(const QList<QString> &object_classes) const;

    QHash<QString, QString> right_to_guid_map;
    QHash<QByteArray, QString> rights_guid_to_name_map;
    QHash<QString, QByteArray> rights_name_to_guid_map;
    QHash<QByteArray, QList<QString>> rights_applies_to_map;

    QHash<QByteArray, QString> guid_to_attribute_map;
    QHash<QByteArray, QString> guid_to_class_map;

    QList<QString> supported_control_list;
};

#endif /* AD_CONFIG_P_H */
