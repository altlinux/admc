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

#ifndef AD_CONFIG_H
#define AD_CONFIG_H

/**
 * Provides access to constant server data, which includes
 * configuration data and some DN's. All of the data is
 * loaded once to avoid unnecessary server requests.
 */

#include "ad_defines.h"

class AdConfigPrivate;
class AdInterface;
class QLocale;
class QString;
class QLineEdit;
template <typename T> class QList;

// NOTE: name strings to reduce confusion
typedef QString ObjectClass;
typedef QString Attribute;

class AdConfig {
    
public:
    AdConfig();
    ~AdConfig();
    
    void load(AdInterface &ad, const QLocale &locale);

    QString domain() const;
    QString domain_head() const;
    QString configuration_dn() const;
    QString schema_dn() const;
    QString partitions_dn() const;

    QString get_attribute_display_name(const Attribute &attribute, const ObjectClass &objectClass) const;

    QString get_class_display_name(const ObjectClass &objectClass) const;

    QList<Attribute> get_columns() const;
    QString get_column_display_name(const Attribute &attribute) const;
    int get_column_index(const QString &attribute) const;

    QList<ObjectClass> get_filter_containers() const;

    QList<ObjectClass> get_possible_superiors(const QList<ObjectClass> &object_classes) const;

    QList<Attribute> get_optional_attributes(const QList<ObjectClass> &object_classes) const;
    QList<Attribute> get_mandatory_attributes(const QList<ObjectClass> &object_classes) const;
    QList<Attribute> get_find_attributes(const ObjectClass &object_class) const;

    AttributeType get_attribute_type(const Attribute &attribute) const;
    LargeIntegerSubtype get_attribute_large_integer_subtype(const Attribute &attribute) const;
    bool get_attribute_is_number(const Attribute &attribute) const;
    bool get_attribute_is_single_valued(const Attribute &attribute) const;
    bool get_attribute_is_system_only(const Attribute &attribute) const;
    int get_attribute_range_upper(const Attribute &attribute) const;
    bool get_attribute_is_backlink(const Attribute &attribute) const;
    bool get_attribute_is_constructed(const Attribute &attribute) const;

    void limit_edit(QLineEdit *edit, const QString &attribute);

    QList<QString> get_noncontainer_classes();

private:
    AdConfigPrivate *d;
};

#endif /* AD_CONFIG_H */
