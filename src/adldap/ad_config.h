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

#ifndef AD_CONFIG_H
#define AD_CONFIG_H

/**
 * Provides access to constant server data, which includes
 * configuration data and some DN's. All of the data is
 * loaded once to avoid unnecessary server requests.
 */

#include "ad_defines.h"

#include <QLocale>

class AdConfigPrivate;
class AdInterface;
class QLocale;
class QString;
class QLineEdit;
class QByteArray;
template <typename T>
class QList;

// NOTE: name strings to reduce confusion
typedef QString ObjectClass;
typedef QString Attribute;

class AdConfig {

public:
    AdConfig();
    ~AdConfig();

    void load(AdInterface &ad, const QLocale &locale);

    QString domain() const;
    QString domain_dn() const;
    QString configuration_dn() const;
    QString schema_dn() const;
    QString partitions_dn() const;
    QString extended_rights_dn() const;
    QString policies_dn() const;
    bool control_is_supported(const QString &control_oid) const;
    QString domain_sid() const;
    QString root_domain_dn() const;

    QString get_attribute_display_name(const Attribute &attribute, const ObjectClass &objectClass) const;

    QString get_class_display_name(const ObjectClass &objectClass) const;

    QList<Attribute> get_columns() const;
    QString get_column_display_name(const Attribute &attribute) const;
    int get_column_index(const QString &attribute) const;

    QList<ObjectClass> get_filter_containers() const;

    QList<ObjectClass> get_possible_superiors(const QList<ObjectClass> &object_classes) const;
    ObjectClass get_parent_class(const ObjectClass &object_class) const;
    // Returns all ancestors of given class and the
    // given class itself
    QList<ObjectClass> get_inherit_chain(const ObjectClass &object_class) const;

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

    // Limit's edit's max valid input length based on
    // the upper range defined for attribute in schema
    void limit_edit(QLineEdit *edit, const QString &attribute);

    QByteArray get_right_guid(const QString &right_cn) const;
    QString get_right_name(const QByteArray &right_guid, const QLocale::Language language) const;
    int get_rights_valid_accesses(const QString &rights_cn) const;

    // Returns extended rights that apply to given
    // classes
    QList<QString> get_extended_rights_list(const QList<QString> &class_list) const;

    QString guid_to_attribute(const QByteArray &guid) const;
    QByteArray attribute_to_guid(const QString &attr) const;

    QString guid_to_class(const QByteArray &guid) const;

    QList<QString> get_noncontainer_classes();

    bool rights_applies_to_class(const QString &rights_cn, const QList<QString> &class_list) const;

    QStringList get_possible_inferiors(const QString &obj_class) const;
    QStringList get_permissionable_attributes(const QString &obj_class) const;

    QByteArray guid_from_class(const ObjectClass &object_class);

    bool class_is_auxiliary(const QString &obj_class) const;

    // Gets all children class chain for given class
    QList<QString> all_inferiors_list(const QString &obj_class) const;

    // Gets all classes, for which there are extended rights
    QList<QString> all_extended_right_classes() const;

private:
    void load_extended_rights(AdInterface &ad);
    void load_attribute_schemas(AdInterface &ad);
    void load_class_schemas(AdInterface &ad);

    // Loads class and attribute display names
    // NOTE: can't just store objects for these because the values require a decent amount of preprocessing which is best done once here, not everytime value is requested
    void load_display_names(AdInterface &ad, const QString &locale_dir);

    void load_columns(AdInterface &ad, const QString &locale_dir);
    void load_filter_containers(AdInterface &ad, const QString &locale_dir);

    void load_permissionable_attributes(const QString &obj_class, AdInterface &ad);

    AdConfigPrivate *d;
};

#endif /* AD_CONFIG_H */
