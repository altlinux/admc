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
    QString domain_sid;
    QString root_domain_dn;

    QList<ObjectClass> filter_containers;

    QList<Attribute> columns;
    QHash<Attribute, QString> column_display_names;

    QHash<ObjectClass, QString> class_display_names;
    QHash<ObjectClass, QList<Attribute>> find_attributes;
    QHash<ObjectClass, QHash<Attribute, QString>> attribute_display_names;

    QHash<Attribute, AdObject> attribute_schemas;
    QHash<ObjectClass, AdObject> class_schemas;

    QList<ObjectClass> add_auxiliary_classes(const QList<QString> &object_classes) const;

    QHash<QString, QByteArray> right_to_guid_map;
    QHash<QByteArray, QString> right_guid_to_cn_map;
    QHash<QByteArray, QString> rights_guid_to_name_map;
    QHash<QString, QByteArray> rights_name_to_guid_map;
    QHash<QByteArray, QList<QString>> rights_applies_to_map;
    QList<QString> extended_rights_list;
    QHash<QString, int> rights_valid_accesses_map;

    QHash<QByteArray, QString> guid_to_attribute_map;
    QHash<QByteArray, QString> guid_to_class_map;

    QList<QString> supported_control_list;

    QHash<QString, QString> sub_class_of_map;

    // Contains classes of possible child objects for given container class.
    // This also includes possible child classes of child classes (and etc).
    QHash<QString, QStringList> class_possible_inferiors_map;
    // Contains editable attributes for the object class and its child classes.
    // Used when assigning custom permissions.
    QHash<QString, QStringList> class_permissionable_attributes_map;
};

#endif /* AD_CONFIG_P_H */
