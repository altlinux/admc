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
    AttributeType_Printable,
    AttributeType_Sid,
    AttributeType_Teletex,
    AttributeType_Unicode,
    AttributeType_UTCTime,
    AttributeType_GeneralizedTime,
};

// NOTE: large integer type has sub types but AD schema
// doesn't distinguish between them (from what I've seen).
// Create enums for subtypes for easier processing.
enum LargeIntegerSubtype {
    LargeIntegerSubtype_Integer,
    LargeIntegerSubtype_Datetime,
    LargeIntegerSubtype_Timespan,
};

// Provides access to some server configuration data
// NOTE: it is assumed that a language change requires a restart
// so localized data is loaded once and is then reused after that

class AdConfig final : public QObject {
Q_OBJECT

public:
    AdConfig(QObject *parent);

    QString get_attribute_display_name(const QString &attribute, const QString &objectClass) const;
    QString get_class_display_name(const QString &objectClass) const;
    QList<QString> get_extra_columns() const;
    QList<QString> get_filter_containers() const;
    QList<QString> get_possible_superiors(const QString &object_category) const;
    QList<QString> get_possible_attributes(const QList<QString> &object_classes) const;
    QList<QString> get_find_attributes(const QString &object_class) const;
    AttributeType get_attribute_type(const QString &attribute) const;
    LargeIntegerSubtype get_large_integer_subtype(const QString &attribute) const;
    bool attribute_is_number(const QString &attribute) const;
    bool get_attribute_is_single_valued(const QString &attribute) const;
    bool get_attribute_is_system_only(const QString &attribute) const;

private:
    // ldap name => ad name
    QHash<QString, QString> ldap_to_ad_names;
    QHash<QString, QString> ad_to_ldap_names;

    // object class => attribute => display name
    QHash<QString, QHash<QString, QString>> attribute_display_names;

    // object class => display name
    QHash<QString, QString> class_display_names;

    QList<QString> extra_columns;
    QList<QString> filter_containers;

    // object category => superiors
    QHash<QString, QList<QString>> possible_superiors;

    // object class => attributes
    QHash<QString, QList<QString>> possible_attributes;

    QHash<QString, QList<QString>> find_attributes;

    // attribute name => type
    QHash<QString, AttributeType> attribute_types;

    QHash<QString, bool> attribute_is_single_valued;
    QHash<QString, bool> attribute_is_system_only;


    QString get_ldap_to_ad_name(const QString &ldap_name) const;
    QString get_ad_to_ldap_name(const QString &ad_name) const;
};

AdConfig *ADCONFIG();

#endif /* SERVER_CONFIGURATION_H */
