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

#ifndef SERVER_CONFIGURATION_H
#define SERVER_CONFIGURATION_H

// Provides access to some server configuration data. All of
// the data is loaded once at startup to avoid unnecessary
// server requests.

#include <QObject>
#include <QString>
#include <QList>
#include <QHash>

enum AttributeType {
    AttributeType_Boolean,
    AttributeType_Enumeration,
    AttributeType_Integer,
    AttributeType_LargeInteger,
    AttributeType_StringCase,
    AttributeType_IA5,
    AttributeType_NTSecDesc,
    AttributeType_Numeric,
    AttributeType_ObjectIdentifier,
    AttributeType_Octet,
    AttributeType_ReplicaLink,
    AttributeType_Printable,
    AttributeType_Sid,
    AttributeType_Teletex,
    AttributeType_Unicode,
    AttributeType_UTCTime,
    AttributeType_GeneralizedTime,
    AttributeType_DNString,
    AttributeType_DNBinary,
    AttributeType_DSDN,
};

// NOTE: large integer type has sub types but AD schema
// doesn't distinguish between them (from what I've seen).
// Create enums for subtypes for easier processing.
enum LargeIntegerSubtype {
    LargeIntegerSubtype_Integer,
    LargeIntegerSubtype_Datetime,
    LargeIntegerSubtype_Timespan,
};

// NOTE: name strings to reduce confusion
typedef QString ObjectClass;
typedef QString Attribute;

class QLineEdit;
class AdObject;

class AdConfig final : public QObject {
Q_OBJECT

public:
    AdConfig(QObject *parent);

    QList<ObjectClass> get_filter_containers() const;

    QList<Attribute> get_columns() const;
    QString get_column_display_name(const Attribute &attribute) const;

    QList<ObjectClass> get_possible_superiors(const QList<ObjectClass> &object_classes) const;

    QString get_class_display_name(const ObjectClass &objectClass) const;
    QList<Attribute> get_possible_attributes(const QList<ObjectClass> &object_classes) const;
    QList<Attribute> get_find_attributes(const ObjectClass &object_class) const;
    QString get_attribute_display_name(const Attribute &attribute, const ObjectClass &objectClass) const;

    AttributeType get_attribute_type(const Attribute &attribute) const;
    bool get_attribute_is_single_valued(const Attribute &attribute) const;
    bool get_attribute_is_system_only(const Attribute &attribute) const;
    int get_attribute_range_upper(const Attribute &attribute) const;
    bool attribute_is_number(const Attribute &attribute) const;
    LargeIntegerSubtype get_large_integer_subtype(const Attribute &attribute) const;

    void limit_edit(QLineEdit *edit, const QString &attribute);

private:
    QList<ObjectClass> filter_containers;
    
    QList<Attribute> columns;
    QHash<Attribute, QString> column_display_names;

    QHash<ObjectClass, QString> class_display_names;
    QHash<ObjectClass, QList<Attribute>> find_attributes;
    QHash<ObjectClass, QHash<Attribute, QString>> attribute_display_names;

    QHash<Attribute, AdObject> attribute_schemas;
    QHash<ObjectClass, AdObject> class_schemas;
};

AdConfig *ADCONFIG();

#endif /* SERVER_CONFIGURATION_H */
