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

#include "ad_config.h"
#include "ad_config_p.h"

#include "ad_filter.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "ad_security.h"
#include "ad_utils.h"
#include "ad_display.h"

#include "samba/ndr_security.h"

#include <QCoreApplication>
#include <QDebug>
#include <QLocale>
#include <algorithm>

#define ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES "attributeDisplayNames"
#define ATTRIBUTE_EXTRA_COLUMNS "extraColumns"
#define ATTRIBUTE_FILTER_CONTAINERS "msDS-FilterContainers"
#define ATTRIBUTE_LDAP_DISPLAY_NAME "lDAPDisplayName"
#define ATTRIBUTE_POSSIBLE_SUPERIORS "possSuperiors"
#define ATTRIBUTE_SYSTEM_POSSIBLE_SUPERIORS "systemPossSuperiors"
#define ATTRIBUTE_ATTRIBUTE_SYNTAX "attributeSyntax"
#define ATTRIBUTE_OM_SYNTAX "oMSyntax"
#define ATTRIBUTE_CLASS_DISPLAY_NAME "classDisplayName"
#define ATTRIBUTE_MAY_CONTAIN "mayContain"
#define ATTRIBUTE_SYSTEM_MAY_CONTAIN "systemMayContain"
#define ATTRIBUTE_MUST_CONTAIN "mustContain"
#define ATTRIBUTE_SYSTEM_MUST_CONTAIN "systemMustContain"
#define ATTRIBUTE_IS_SINGLE_VALUED "isSingleValued"
#define ATTRIBUTE_SYSTEM_ONLY "systemOnly"
#define ATTRIBUTE_RANGE_UPPER "rangeUpper"
#define ATTRIBUTE_AUXILIARY_CLASS "auxiliaryClass"
#define ATTRIBUTE_SYSTEM_FLAGS "systemFlags"
#define ATTRIBUTE_LINK_ID "linkID"
#define ATTRIBUTE_SYSTEM_AUXILIARY_CLASS "systemAuxiliaryClass"
#define ATTRIBUTE_SUB_CLASS_OF "subClassOf"
#define ATTRIBUTE_POSSIBLE_INFERIORS "possibleInferiors"
#define ATTRIBUTE_ALLOWED_ATTRIBUTES "allowedAttributes"
#define ATTRIBUTE_ALLOWED_ATTRIBUTES_EFFECTIVE "allowedAttributesEffective"
#define ATTRIBUTE_OBJECT_CLASS_CATEGORY "objectClassCategory"

#define CLASS_ATTRIBUTE_SCHEMA "attributeSchema"
#define CLASS_CLASS_SCHEMA "classSchema"
#define CLASS_CONTROL_ACCESS_RIGHT "controlAccessRight"

#define FLAG_ATTR_IS_CONSTRUCTED 0x00000004

AdConfigPrivate::AdConfigPrivate() {
}

AdConfig::AdConfig() {
    d = new AdConfigPrivate();
}

AdConfig::~AdConfig() {
    delete d;
}

void AdConfig::load(AdInterface &ad, const QLocale &locale) {
    d->domain = ad.get_domain();

    d->filter_containers.clear();
    d->columns.clear();
    d->column_display_names.clear();
    d->class_display_names.clear();
    d->find_attributes.clear();
    d->attribute_display_names.clear();
    d->attribute_schemas.clear();
    d->class_schemas.clear();

    const AdObject rootDSE_object = ad.search_object(ROOT_DSE);
    d->domain_dn = rootDSE_object.get_string(ATTRIBUTE_DEFAULT_NAMING_CONTEXT);
    d->schema_dn = rootDSE_object.get_string(ATTRIBUTE_SCHEMA_NAMING_CONTEXT);
    d->configuration_dn = rootDSE_object.get_string(ATTRIBUTE_CONFIGURATION_NAMING_CONTEXT);
    d->supported_control_list = rootDSE_object.get_strings(ATTRIBUTE_SUPPORTED_CONTROL);
    d->root_domain_dn = rootDSE_object.get_string(ATTRIBUTE_ROOT_DOMAIN_NAMING_CONTEXT);

    const AdObject domain_object = ad.search_object(domain_dn());
    d->domain_sid = object_sid_display_value(domain_object.get_value(ATTRIBUTE_OBJECT_SID));

    const QString locale_dir = [this, locale]() {
        const QString locale_code = [locale]() {
            if (locale.language() == QLocale::Russian) {
                return "419";
            } else {
                // English
                return "409";
            }
        }();

        return QString("CN=%1,CN=DisplaySpecifiers,%2").arg(locale_code, configuration_dn());
    }();

    load_attribute_schemas(ad);

    load_class_schemas(ad);

    load_display_names(ad, locale_dir);

    load_columns(ad, locale_dir);

    load_filter_containers(ad, locale_dir);

    load_extended_rights(ad);

    load_permissionable_attributes(CLASS_DOMAIN, ad);
}

QString AdConfig::domain() const {
    return d->domain;
}

QString AdConfig::domain_dn() const {
    return d->domain_dn;
}

QString AdConfig::configuration_dn() const {
    return d->configuration_dn;
}

QString AdConfig::schema_dn() const {
    return d->schema_dn;
}

QString AdConfig::partitions_dn() const {
    return QString("CN=Partitions,%1").arg(configuration_dn());
}

QString AdConfig::extended_rights_dn() const {
    return QString("CN=Extended-Rights,%1").arg(configuration_dn());
}

QString AdConfig::policies_dn() const {
    return QString("CN=Policies,CN=System,%1").arg(domain_dn());
}

bool AdConfig::control_is_supported(const QString &control_oid) const {
    const bool supported = d->supported_control_list.contains(control_oid);

    return supported;
}

QString AdConfig::domain_sid() const
{
    return d->domain_sid;
}

QString AdConfig::root_domain_dn() const {
    return d->root_domain_dn;
}

QString AdConfig::get_attribute_display_name(const Attribute &attribute, const ObjectClass &objectClass) const {
    if (d->attribute_display_names.contains(objectClass) && d->attribute_display_names[objectClass].contains(attribute)) {
        const QString display_name = d->attribute_display_names[objectClass][attribute];

        return display_name;
    }

    // NOTE: display specifier doesn't cover all attributes for all classes, so need to hardcode some of them here
    static const QHash<Attribute, QString> fallback_display_names = {
        {ATTRIBUTE_NAME, QCoreApplication::translate("AdConfig", "Name")},
        {ATTRIBUTE_DN, QCoreApplication::translate("AdConfig", "Distinguished name")},
        {ATTRIBUTE_OBJECT_CLASS, QCoreApplication::translate("AdConfig", "Object class")},
        {ATTRIBUTE_WHEN_CREATED, QCoreApplication::translate("AdConfig", "Created")},
        {ATTRIBUTE_WHEN_CHANGED, QCoreApplication::translate("AdConfig", "Changed")},
        {ATTRIBUTE_USN_CREATED, QCoreApplication::translate("AdConfig", "USN created")},
        {ATTRIBUTE_USN_CHANGED, QCoreApplication::translate("AdConfig", "USN changed")},
        {ATTRIBUTE_ACCOUNT_EXPIRES, QCoreApplication::translate("AdConfig", "Account expires")},
        {ATTRIBUTE_OBJECT_CATEGORY, QCoreApplication::translate("AdConfig", "Type")},
        {ATTRIBUTE_PROFILE_PATH, QCoreApplication::translate("AdConfig", "Profile path")},
        {ATTRIBUTE_SCRIPT_PATH, QCoreApplication::translate("AdConfig", "Logon script")},
        {ATTRIBUTE_SAM_ACCOUNT_NAME, QCoreApplication::translate("AdConfig", "Logon name (pre-Windows 2000)")},
        {ATTRIBUTE_MAIL, QCoreApplication::translate("AdConfig", "E-mail")},
        {ATTRIBUTE_LOCATION, QCoreApplication::translate("AdConfig", "Location")},
        {ATTRIBUTE_MANAGED_BY, QCoreApplication::translate("managedBy", "Managed by")},
    };

    return fallback_display_names.value(attribute, attribute);
}

QString AdConfig::get_class_display_name(const QString &objectClass) const {
    return d->class_display_names.value(objectClass, objectClass);
}

QList<QString> AdConfig::get_columns() const {
    return d->columns;
}

QString AdConfig::get_column_display_name(const Attribute &attribute) const {
    return d->column_display_names.value(attribute, attribute);
}

int AdConfig::get_column_index(const QString &attribute) const {
    if (!d->columns.contains(attribute)) {
        qWarning() << "ADCONFIG columns missing attribute:" << attribute;
    }

    return d->columns.indexOf(attribute);
}

QList<QString> AdConfig::get_filter_containers() const {
    return d->filter_containers;
}

QList<QString> AdConfig::get_possible_superiors(const QList<ObjectClass> &object_classes) const {
    QList<QString> out;

    for (const QString &object_class : object_classes) {
        const AdObject schema = d->class_schemas[object_class];
        out += schema.get_strings(ATTRIBUTE_POSSIBLE_SUPERIORS);
        out += schema.get_strings(ATTRIBUTE_SYSTEM_POSSIBLE_SUPERIORS);
    }

    out.removeDuplicates();

    return out;
}

ObjectClass AdConfig::get_parent_class(const ObjectClass &object_class) const {
    const ObjectClass out = d->sub_class_of_map.value(object_class);

    return out;
}

QList<ObjectClass> AdConfig::get_inherit_chain(const ObjectClass &object_class) const {
    QList<QString> out;

    ObjectClass current_class = object_class;

    while (true) {
        out.append(current_class);

        const QString parent_class = get_parent_class(current_class);

        const bool chain_ended = (parent_class == current_class);

        if (chain_ended) {
            break;
        } else {
            current_class = parent_class;
        }
    }

    return out;
}

QList<QString> AdConfig::get_optional_attributes(const QList<QString> &object_classes) const {
    const QList<QString> all_classes = d->add_auxiliary_classes(object_classes);

    QList<QString> attributes;

    for (const auto &object_class : all_classes) {
        const AdObject schema = d->class_schemas[object_class];
        attributes += schema.get_strings(ATTRIBUTE_MAY_CONTAIN);
        attributes += schema.get_strings(ATTRIBUTE_SYSTEM_MAY_CONTAIN);
    }

    attributes.removeDuplicates();

    return attributes;
}

QList<QString> AdConfig::get_mandatory_attributes(const QList<QString> &object_classes) const {
    const QList<QString> all_classes = d->add_auxiliary_classes(object_classes);

    QList<QString> attributes;

    for (const auto &object_class : all_classes) {
        const AdObject schema = d->class_schemas[object_class];
        attributes += schema.get_strings(ATTRIBUTE_MUST_CONTAIN);
        attributes += schema.get_strings(ATTRIBUTE_SYSTEM_MUST_CONTAIN);
    }

    attributes.removeDuplicates();

    return attributes;
}

QList<QString> AdConfig::get_find_attributes(const QString &object_class) const {
    return d->find_attributes.value(object_class, QList<QString>());
}

AttributeType AdConfig::get_attribute_type(const QString &attribute) const {
    // NOTE: replica of: https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-adts/7cda533e-d7a4-4aec-a517-91d02ff4a1aa
    // syntax -> om syntax list -> type
    static QHash<QString, QHash<QString, AttributeType>> type_map = {
        {"2.5.5.8", {{"1", AttributeType_Boolean}}},
        {"2.5.5.9",
            {
                {"10", AttributeType_Enumeration},
                {"2", AttributeType_Integer},
            }},
        {"2.5.5.16", {{"65", AttributeType_LargeInteger}}},
        {"2.5.5.3", {{"27", AttributeType_StringCase}}},
        {"2.5.5.5", {{"22", AttributeType_IA5}}},
        {"2.5.5.15", {{"66", AttributeType_NTSecDesc}}},
        {"2.5.5.6", {{"18", AttributeType_Numeric}}},
        {"2.5.5.2", {{"6", AttributeType_ObjectIdentifier}}},
        {"2.5.5.10",
            {
                {"4", AttributeType_Octet},
                {"127", AttributeType_ReplicaLink},
            }},
        {"2.5.5.5", {{"19", AttributeType_Printable}}},
        {"2.5.5.17", {{"4", AttributeType_Sid}}},
        {"2.5.5.4", {{"20", AttributeType_Teletex}}},
        {"2.5.5.12", {{"64", AttributeType_Unicode}}},
        {"2.5.5.11",
            {
                {"23", AttributeType_UTCTime},
                {"24", AttributeType_GeneralizedTime},
            }},
        {"2.5.5.14", {{"127", AttributeType_DNString}}},
        {"2.5.5.7", {{"127", AttributeType_DNBinary}}},
        {"2.5.5.1", {{"127", AttributeType_DSDN}}},
    };

    const AdObject schema = d->attribute_schemas[attribute];

    const QString attribute_syntax = schema.get_string(ATTRIBUTE_ATTRIBUTE_SYNTAX);
    const QString om_syntax = schema.get_string(ATTRIBUTE_OM_SYNTAX);
    if (type_map.contains(attribute_syntax) && type_map[attribute_syntax].contains(om_syntax)) {
        return type_map[attribute_syntax][om_syntax];
    } else {
        return AttributeType_StringCase;
    }
}

LargeIntegerSubtype AdConfig::get_attribute_large_integer_subtype(const QString &attribute) const {
    // Manually remap large integer types to subtypes
    static const QList<QString> datetimes = {
        ATTRIBUTE_ACCOUNT_EXPIRES,
        ATTRIBUTE_LAST_LOGON,
        ATTRIBUTE_LAST_LOGON_TIMESTAMP,
        ATTRIBUTE_PWD_LAST_SET,
        ATTRIBUTE_BAD_PWD_TIME,
        ATTRIBUTE_CREATION_TIME,
        ATTRIBUTE_MSDS_USER_PASSWORD_EXPIRY_TIME_COMPUTED,
        ATTRIBUTE_LAPS_V2_EXPIRATION_TIME,
    };
    static const QList<QString> timespans = {
        ATTRIBUTE_MAX_PWD_AGE,
        ATTRIBUTE_MIN_PWD_AGE,
        ATTRIBUTE_LOCKOUT_DURATION,
        ATTRIBUTE_LOCKOUT_OBSERVATION_WINDOW,
        ATTRIBUTE_FORCE_LOGOFF,
        ATTRIBUTE_MS_DS_LOCKOUT_DURATION,
        ATTRIBUTE_MS_DS_LOCKOUT_OBSERVATION_WINDOW,
        ATTRIBUTE_MS_DS_MAX_PASSWORD_AGE,
        ATTRIBUTE_MS_DS_MIN_PASSWORD_AGE
    };

    if (datetimes.contains(attribute)) {
        return LargeIntegerSubtype_Datetime;
    } else if (timespans.contains(attribute)) {
        return LargeIntegerSubtype_Timespan;
    } else {
        return LargeIntegerSubtype_Integer;
    }
}

bool AdConfig::get_attribute_is_number(const QString &attribute) const {
    static const QList<AttributeType> number_types = {
        AttributeType_Integer,
        AttributeType_LargeInteger,
        AttributeType_Enumeration,
        AttributeType_Numeric,
    };
    const AttributeType type = get_attribute_type(attribute);

    return number_types.contains(type);
}

bool AdConfig::get_attribute_is_single_valued(const QString &attribute) const {
    return d->attribute_schemas[attribute].get_bool(ATTRIBUTE_IS_SINGLE_VALUED);
}

bool AdConfig::get_attribute_is_system_only(const QString &attribute) const {
    return d->attribute_schemas[attribute].get_bool(ATTRIBUTE_SYSTEM_ONLY);
}

int AdConfig::get_attribute_range_upper(const QString &attribute) const {
    return d->attribute_schemas[attribute].get_int(ATTRIBUTE_RANGE_UPPER);
}

bool AdConfig::get_attribute_is_backlink(const QString &attribute) const {
    if (d->attribute_schemas[attribute].contains(ATTRIBUTE_LINK_ID)) {
        const int link_id = d->attribute_schemas[attribute].get_int(ATTRIBUTE_LINK_ID);
        const bool link_id_is_odd = (link_id % 2 != 0);

        return link_id_is_odd;
    } else {
        return false;
    }
}

bool AdConfig::get_attribute_is_constructed(const QString &attribute) const {
    const int system_flags = d->attribute_schemas[attribute].get_int(ATTRIBUTE_SYSTEM_FLAGS);
    return bitmask_is_set(system_flags, FLAG_ATTR_IS_CONSTRUCTED);
}

QByteArray AdConfig::get_right_guid(const QString &right_cn) const {
    const QByteArray out = d->right_to_guid_map.value(right_cn, QByteArray());
    return out;
}

// NOTE: technically, Active Directory provides
// translations for right names but it's not
// accessible, so have to translate these ourselves. On
// Windows, you would use the localizationDisplayId
// retrieved from schema to get translation from
// dssec.dll. And we don't have dssec.dll, nor do we
// have the ability to interact with it!
QString AdConfig::get_right_name(const QByteArray &right_guid, const QLocale::Language language) const {
    const QHash<QString, QString> cn_to_map_russian = {
        {"DS-Replication-Get-Changes", QCoreApplication::translate("AdConfig", "DS Replication Get Changes")},
        {"DS-Replication-Get-Changes-All", QCoreApplication::translate("AdConfig", "DS Replication Get Changes All")},
        {"Email-Information", QCoreApplication::translate("AdConfig", "Phone and Mail Options")},
        {"DS-Bypass-Quota", QCoreApplication::translate("AdConfig", "Bypass the quota restrictions during creation.")},
        {"Receive-As", QCoreApplication::translate("AdConfig", "Receive As")},
        {"Unexpire-Password", QCoreApplication::translate("AdConfig", "Unexpire Password")},
        {"Do-Garbage-Collection", QCoreApplication::translate("AdConfig", "Do Garbage Collection")},
        {"Allowed-To-Authenticate", QCoreApplication::translate("AdConfig", "Allowed To Authenticate")},
        {"Change-PDC", QCoreApplication::translate("AdConfig", "Change PDC")},
        {"Reanimate-Tombstones", QCoreApplication::translate("AdConfig", "Reanimate Tombstones")},
        {"msmq-Peek-Dead-Letter", QCoreApplication::translate("AdConfig", "msmq Peek Dead Letter")},
        {"Certificate-AutoEnrollment", QCoreApplication::translate("AdConfig", "AutoEnrollment")},
        {"DS-Install-Replica", QCoreApplication::translate("AdConfig", "DS Install Replica")},
        {"Domain-Password", QCoreApplication::translate("AdConfig", "Domain Password & Lockout Policies")},
        {"Generate-RSoP-Logging", QCoreApplication::translate("AdConfig", "Generate RSoP Logging")},
        {"Run-Protect-Admin-Groups-Task", QCoreApplication::translate("AdConfig", "Run Protect Admin Groups Task")},
        {"Self-Membership", QCoreApplication::translate("AdConfig", "Self Membership")},
        {"DS-Clone-Domain-Controller", QCoreApplication::translate("AdConfig", "Allow a DC to create a clone of itself")},
        {"Domain-Other-Parameters", QCoreApplication::translate("AdConfig", "Other Domain Parameters (for use by SAM)")},
        {"SAM-Enumerate-Entire-Domain", QCoreApplication::translate("AdConfig", "SAM Enumerate Entire Domain")},
        {"DS-Write-Partition-Secrets", QCoreApplication::translate("AdConfig", "Write secret attributes of objects in a Partition")},
        {"Send-As", QCoreApplication::translate("AdConfig", "Send As")},
        {"DS-Replication-Manage-Topology", QCoreApplication::translate("AdConfig", "DS Replication Manage Topology")},
        {"DS-Set-Owner", QCoreApplication::translate("AdConfig", "Set Owner of an object during creation.")},
        {"Generate-RSoP-Planning", QCoreApplication::translate("AdConfig", "Generate RSoP Planning")},
        {"Certificate-Enrollment", QCoreApplication::translate("AdConfig", "Certificate Enrollment")},
        {"Web-Information", QCoreApplication::translate("AdConfig", "Web Information")},
        {"Create-Inbound-Forest-Trust", QCoreApplication::translate("AdConfig", "Create Inbound Forest Trust")},
        {"Migrate-SID-History", QCoreApplication::translate("AdConfig", "Migrate SID History")},
        {"Update-Password-Not-Required-Bit", QCoreApplication::translate("AdConfig", "Update Password Not Required Bit")},
        {"MS-TS-GatewayAccess", QCoreApplication::translate("AdConfig", "MS-TS-GatewayAccess")},
        {"Validated-MS-DS-Additional-DNS-Host-Name", QCoreApplication::translate("AdConfig", "Validated write to MS DS Additional DNS Host Name")},
        {"msmq-Receive", QCoreApplication::translate("AdConfig", "msmq Receive")},
        {"Validated-DNS-Host-Name", QCoreApplication::translate("AdConfig", "Validated DNS Host Name")},
        {"Send-To", QCoreApplication::translate("AdConfig", "Send To")},
        {"DS-Replication-Get-Changes-In-Filtered-Set", QCoreApplication::translate("AdConfig", "DS Replication Get Changes In Filtered Set")},
        {"Read-Only-Replication-Secret-Synchronization", QCoreApplication::translate("AdConfig", "Read Only Replication Secret Synchronization")},
        {"Validated-MS-DS-Behavior-Version", QCoreApplication::translate("AdConfig", "Validated write to MS DS behavior version")},
        {"msmq-Open-Connector", QCoreApplication::translate("AdConfig", "msmq Open Connector")},
        {"Terminal-Server-License-Server", QCoreApplication::translate("AdConfig", "Terminal Server License Server")},
        {"Change-Schema-Master", QCoreApplication::translate("AdConfig", "Change Schema Master")},
        {"Recalculate-Hierarchy", QCoreApplication::translate("AdConfig", "Recalculate Hierarchy")},
        {"DS-Check-Stale-Phantoms", QCoreApplication::translate("AdConfig", "DS Check Stale Phantoms")},
        {"msmq-Receive-computer-Journal", QCoreApplication::translate("AdConfig", "msmq Receive computer Journal")},
        {"User-Force-Change-Password", QCoreApplication::translate("AdConfig", "User Force Change Password")},
        {"Domain-Administer-Server", QCoreApplication::translate("AdConfig", "Domain Administer Server")},
        {"DS-Replication-Synchronize", QCoreApplication::translate("AdConfig", "DS Replication Synchronize")},
        {"Personal-Information", QCoreApplication::translate("AdConfig", "Personal Information")},
        {"msmq-Peek", QCoreApplication::translate("AdConfig", "msmq Peek")},
        {"General-Information", QCoreApplication::translate("AdConfig", "General Information")},
        {"Membership", QCoreApplication::translate("AdConfig", "Group Membership")},
        {"Add-GUID", QCoreApplication::translate("AdConfig", "Add GUID")},
        {"RAS-Information", QCoreApplication::translate("AdConfig", "Remote Access Information")},
        {"DS-Execute-Intentions-Script", QCoreApplication::translate("AdConfig", "DS Execute Intentions Script")},
        {"Allocate-Rids", QCoreApplication::translate("AdConfig", "Allocate Rids")},
        {"Update-Schema-Cache", QCoreApplication::translate("AdConfig", "Update Schema Cache")},
        {"Apply-Group-Policy", QCoreApplication::translate("AdConfig", "Apply Group Policy")},
        {"User-Account-Restrictions", QCoreApplication::translate("AdConfig", "Account Restrictions")},
        {"Validated-SPN", QCoreApplication::translate("AdConfig", "Validated SPN")},
        {"DS-Read-Partition-Secrets", QCoreApplication::translate("AdConfig", "Read secret attributes of objects in a Partition")},
        {"User-Logon", QCoreApplication::translate("AdConfig", "Logon Information")},
        {"DS-Query-Self-Quota", QCoreApplication::translate("AdConfig", "DS Query Self Quota")},
        {"Change-Infrastructure-Master", QCoreApplication::translate("AdConfig", "Change Infrastructure Master")},
        {"Open-Address-Book", QCoreApplication::translate("AdConfig", "Open Address Book")},
        {"User-Change-Password", QCoreApplication::translate("AdConfig", "User Change Password")},
        {"msmq-Peek-computer-Journal", QCoreApplication::translate("AdConfig", "msmq Peek computer Journal")},
        {"Change-Domain-Master", QCoreApplication::translate("AdConfig", "Change Domain Master")},
        {"msmq-Send", QCoreApplication::translate("AdConfig", "msmq Send")},
        {"Change-Rid-Master", QCoreApplication::translate("AdConfig", "Change Rid Master")},
        {"Recalculate-Security-Inheritance", QCoreApplication::translate("AdConfig", "Recalculate Security Inheritance")},
        {"Refresh-Group-Cache", QCoreApplication::translate("AdConfig", "Refresh Group Cache")},
        {"Manage-Optional-Features", QCoreApplication::translate("AdConfig", "Manage Optional Features")},
        {"Reload-SSL-Certificate", QCoreApplication::translate("AdConfig", "Reload SSL Certificate")},
        {"Enable-Per-User-Reversibly-Encrypted-Password", QCoreApplication::translate("AdConfig", "Enable Per User Reversibly Encrypted Password")},
        {"DS-Replication-Monitor-Topology", QCoreApplication::translate("AdConfig", "DS Replication Monitor Topology")},
        {"Public-Information", QCoreApplication::translate("AdConfig", "Public Information")},
        {"Private-Information", QCoreApplication::translate("AdConfig", "Private Information")},
        {"msmq-Receive-Dead-Letter", QCoreApplication::translate("AdConfig", "msmq Receive Dead Letter")},
        {"msmq-Receive-journal", QCoreApplication::translate("AdConfig", "msmq Receive journal")},
        {"DNS-Host-Name-Attributes", QCoreApplication::translate("AdConfig", "DNS Host Name Attributes")},
    };

    const QString right_cn = d->right_guid_to_cn_map[right_guid];
    if (language == QLocale::Russian && cn_to_map_russian.contains(right_cn)) {
        const QString out = cn_to_map_russian[right_cn];

        return out;
    }

    const QString out = d->rights_guid_to_name_map.value(right_guid, QCoreApplication::translate("AdConfig", "<unknown rights>"));
    return out;
}

QList<QString> AdConfig::get_extended_rights_list(const QList<QString> &class_list) const {
    QList<QString> out;

    for (const QString &rights : d->extended_rights_list) {
        const bool applies_to = rights_applies_to_class(rights, class_list);
        if (applies_to) {
            out.append(rights);
        }
    }

    return out;
}

int AdConfig::get_rights_valid_accesses(const QString &rights_cn) const {
    // NOTE: awkward exception. Can't write group
    // membership because target attribute is
    // constructed. For some reason valid accesses for
    // membership right does allow writing.
    if (rights_cn == "Membership") {
        return SEC_ADS_READ_PROP;
    }

    const int out = d->rights_valid_accesses_map.value(rights_cn, 0);

    return out;
}

QString AdConfig::guid_to_attribute(const QByteArray &guid) const {
    const QString out = d->guid_to_attribute_map.value(guid, "<unknown attribute>");
    return out;
}

QByteArray AdConfig::attribute_to_guid(const QString &attr) const {
    const QByteArray attr_guid = d->guid_to_attribute_map.key(attr, QByteArray());
    return attr_guid;
}

QString AdConfig::guid_to_class(const QByteArray &guid) const {
    const QString out = d->guid_to_class_map.value(guid, "<unknown class>");
    return out;
}

// (noncontainer classes) = (all classes) - (container classes)
QList<QString> AdConfig::get_noncontainer_classes() {
    QList<QString> out = filter_classes;

    const QList<QString> container_classes = get_filter_containers();
    for (const QString &container_class : container_classes) {
        out.removeAll(container_class);
    }

    return out;
}

bool AdConfig::rights_applies_to_class(const QString &rights_cn, const QList<QString> &class_list) const {
    const QByteArray rights_guid = d->rights_name_to_guid_map[rights_cn];

    const QList<QString> applies_to_list = d->rights_applies_to_map[rights_guid];
    const QSet<QString> applies_to_set = QSet<QString>(applies_to_list.begin(), applies_to_list.end());

    const QSet<QString> class_set = QSet<QString>(class_list.begin(), class_list.end());

    const bool applies = applies_to_set.intersects(class_set);

    return applies;
}

QStringList AdConfig::get_possible_inferiors(const QString &obj_class) const {
    return d->class_possible_inferiors_map[obj_class];
}

QStringList AdConfig::get_permissionable_attributes(const QString &obj_class) const {
    return d->class_permissionable_attributes_map[obj_class];
}

QByteArray AdConfig::guid_from_class(const ObjectClass &object_class) {
    return d->guid_to_class_map.key(object_class, QByteArray());
}

bool AdConfig::class_is_auxiliary(const QString &obj_class) const {
    const int auxiliary_category_value = 3;
    const int class_category = d->class_schemas[obj_class].get_int(ATTRIBUTE_OBJECT_CLASS_CATEGORY);
    return class_category == auxiliary_category_value;
}

QList<QString> AdConfig::all_inferiors_list(const QString &obj_class) const {
    QList<QString> out;
    for (const QString &inferior_class : get_possible_inferiors(obj_class)) {
        out.append(inferior_class);
        out.append(get_possible_inferiors(inferior_class));
    }
    // Remove duplicates with converting to set and back
    out = QSet<QString>(out.begin(), out.end()).values();

    return out;
}

QList<QString> AdConfig::all_extended_right_classes() const {
    QList<QString> out;
    for (auto obj_classes : d->rights_applies_to_map.values()) {
        out.append(obj_classes);
    }
    // Remove duplicates with QSet
    out = QSet<QString>(out.begin(), out.end()).values();
    return out;
}

void AdConfig::load_extended_rights(AdInterface &ad)
{
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_CONTROL_ACCESS_RIGHT);

    const QList<QString> attributes = {
        ATTRIBUTE_CN,
        ATTRIBUTE_DISPLAY_NAME,
        ATTRIBUTE_RIGHTS_GUID,
        ATTRIBUTE_APPLIES_TO,
        ATTRIBUTE_VALID_ACCESSES,
    };

    const QString search_base = extended_rights_dn();

    const QHash<QString, AdObject> search_results = ad.search(search_base, SearchScope_Children, filter, attributes);

    for (const AdObject &object : search_results.values()) {
        const QString cn = object.get_string(ATTRIBUTE_CN);
        const QString guid_string = object.get_string(ATTRIBUTE_RIGHTS_GUID);
        const QByteArray guid = guid_string_to_bytes(guid_string);
        const QByteArray display_name = object.get_value(ATTRIBUTE_DISPLAY_NAME);
        const QList<QString> applies_to = [this, object]() {
            QList<QString> out;

            const QList<QString> class_guid_string_list = object.get_strings(ATTRIBUTE_APPLIES_TO);
            for (const QString &class_guid_string : class_guid_string_list) {
                const QByteArray class_guid = guid_string_to_bytes(class_guid_string);
                const QString object_class = guid_to_class(class_guid);

                out.append(object_class);
            }

            return out;
        }();
        const int valid_accesses = object.get_int(ATTRIBUTE_VALID_ACCESSES);

        d->right_to_guid_map[cn] = guid;
        d->right_guid_to_cn_map[guid] = cn;
        d->rights_guid_to_name_map[guid] = display_name;
        d->rights_name_to_guid_map[cn] = guid;
        d->rights_applies_to_map[guid] = applies_to;
        d->extended_rights_list.append(cn);
        d->rights_valid_accesses_map[cn] = valid_accesses;
    }
}

void AdConfig::load_attribute_schemas(AdInterface &ad) {
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_ATTRIBUTE_SCHEMA);

    const QList<QString> attributes = {
        ATTRIBUTE_LDAP_DISPLAY_NAME,
        ATTRIBUTE_ATTRIBUTE_SYNTAX,
        ATTRIBUTE_OM_SYNTAX,
        ATTRIBUTE_IS_SINGLE_VALUED,
        ATTRIBUTE_SYSTEM_ONLY,
        ATTRIBUTE_RANGE_UPPER,
        ATTRIBUTE_LINK_ID,
        ATTRIBUTE_SYSTEM_FLAGS,
        ATTRIBUTE_SCHEMA_ID_GUID,
    };

    const QHash<QString, AdObject> results = ad.search(schema_dn(), SearchScope_Children, filter, attributes);

    for (const AdObject &object : results.values()) {
        const QString attribute = object.get_string(ATTRIBUTE_LDAP_DISPLAY_NAME);
        d->attribute_schemas[attribute] = object;

        const QByteArray guid = object.get_value(ATTRIBUTE_SCHEMA_ID_GUID);
        d->guid_to_attribute_map[guid] = attribute;
    }
}

void AdConfig::load_class_schemas(AdInterface &ad) {
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_CLASS_SCHEMA);

    const QList<QString> attributes = {
        ATTRIBUTE_LDAP_DISPLAY_NAME,
        ATTRIBUTE_POSSIBLE_SUPERIORS,
        ATTRIBUTE_SYSTEM_POSSIBLE_SUPERIORS,
        ATTRIBUTE_MAY_CONTAIN,
        ATTRIBUTE_SYSTEM_MAY_CONTAIN,
        ATTRIBUTE_MUST_CONTAIN,
        ATTRIBUTE_SYSTEM_MUST_CONTAIN,
        ATTRIBUTE_AUXILIARY_CLASS,
        ATTRIBUTE_SYSTEM_AUXILIARY_CLASS,
        ATTRIBUTE_SCHEMA_ID_GUID,
        ATTRIBUTE_SUB_CLASS_OF,
        ATTRIBUTE_SCHEMA_ID_GUID,
        ATTRIBUTE_POSSIBLE_INFERIORS
    };

    const QHash<QString, AdObject> results = ad.search(schema_dn(), SearchScope_Children, filter, attributes);

    for (const AdObject &object : results.values()) {
        const QString object_class = object.get_string(ATTRIBUTE_LDAP_DISPLAY_NAME);
        d->class_schemas[object_class] = object;

        const QByteArray guid = object.get_value(ATTRIBUTE_SCHEMA_ID_GUID);
        d->guid_to_class_map[guid] = object_class;

        const QString sub_class_of = object.get_string(ATTRIBUTE_SUB_CLASS_OF);
        d->sub_class_of_map[object_class] = sub_class_of;

        const QStringList possible_inferiors = bytearray_list_to_string_list(object.get_values(ATTRIBUTE_POSSIBLE_INFERIORS));
        d->class_possible_inferiors_map[object_class] = possible_inferiors;
    }
}

void AdConfig::load_display_names(AdInterface &ad, const QString &locale_dir) {
    const QString filter = QString();

    const QList<QString> search_attributes = {
        ATTRIBUTE_CLASS_DISPLAY_NAME,
        ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES,
    };

    const QHash<QString, AdObject> results = ad.search(locale_dir, SearchScope_Children, filter, search_attributes);

    for (const AdObject &object : results) {
        const QString dn = object.get_dn();

        // Display specifier DN is "CN=object-class-Display,CN=..."
        // Get "object-class" from that
        const QString object_class = [dn]() {
            const QString rdn = dn.split(",")[0];
            QString out = rdn;
            out.remove("CN=", Qt::CaseInsensitive);
            out.remove("-Display");

            return out;
        }();

        if (object.contains(ATTRIBUTE_CLASS_DISPLAY_NAME)) {
            d->class_display_names[object_class] = object.get_string(ATTRIBUTE_CLASS_DISPLAY_NAME);
        }

        if (object.contains(ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES)) {
            const QList<QString> display_names = object.get_strings(ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES);

            for (const auto &display_name_pair : display_names) {
                const QList<QString> split = display_name_pair.split(",");
                const QString attribute_name = split[0];
                const QString display_name = split[1];

                d->attribute_display_names[object_class][attribute_name] = display_name;
            }

            d->find_attributes[object_class] = [object_class, display_names]() {
                QList<QString> out;

                for (const auto &display_name_pair : display_names) {
                    const QList<QString> split = display_name_pair.split(",");
                    const QString attribute = split[0];

                    out.append(attribute);
                }

                return out;
            }();
        }
    }
}

void AdConfig::load_columns(AdInterface &ad, const QString &locale_dir) {
    const QList<QString> columns_values = [&] {
        const QString dn = QString("CN=default-Display,%1").arg(locale_dir);
        const AdObject object = ad.search_object(dn, {ATTRIBUTE_EXTRA_COLUMNS});

        // NOTE: order as stored in attribute is reversed. Order is not sorted alphabetically so can't just sort.
        QList<QString> extra_columns = object.get_strings(ATTRIBUTE_EXTRA_COLUMNS);
        std::reverse(extra_columns.begin(), extra_columns.end());

        return extra_columns;
    }();

    // ATTRIBUTE_EXTRA_COLUMNS value is
    // "$attribute,$display_name,..."
    // Get attributes out of that
    for (const QString &value : columns_values) {
        const QList<QString> column_split = value.split(',');

        if (column_split.size() < 2) {
            continue;
        }

        const QString attribute = column_split[0];
        const QString attribute_display_name = column_split[1];

        d->columns.append(attribute);
        d->column_display_names[attribute] = attribute_display_name;
    }

    // Insert some columns manually
    auto add_custom = [=](const Attribute &attribute, const QString &display_name) {
        d->columns.prepend(attribute);
        d->column_display_names[attribute] = display_name;
    };

    add_custom(ATTRIBUTE_DN, QCoreApplication::translate("AdConfig", "Distinguished name"));
    add_custom(ATTRIBUTE_DESCRIPTION, QCoreApplication::translate("AdConfig", "Description"));
    add_custom(ATTRIBUTE_OBJECT_CLASS, QCoreApplication::translate("AdConfig", "Class"));
    add_custom(ATTRIBUTE_NAME, QCoreApplication::translate("AdConfig", "Name"));
}

void AdConfig::load_filter_containers(AdInterface &ad, const QString &locale_dir) {
    const QString ui_settings_dn = QString("CN=DS-UI-Default-Settings,%1").arg(locale_dir);
    const AdObject object = ad.search_object(ui_settings_dn, {ATTRIBUTE_FILTER_CONTAINERS});

    // NOTE: dns-Zone category is mispelled in
    // ATTRIBUTE_FILTER_CONTAINERS, no idea why, might
    // just be on this domain version
    const QList<QString> categories = [object]() {
        QList<QString> categories_out = object.get_strings(ATTRIBUTE_FILTER_CONTAINERS);
        categories_out.replaceInStrings("dns-Zone", "Dns-Zone");

        return categories_out;
    }();

    // NOTE: ATTRIBUTE_FILTER_CONTAINERS contains object
    // *categories* not classes, so need to get object
    // class from category object
    for (const auto &object_category : categories) {
        const QString category_dn = QString("CN=%1,%2").arg(object_category, schema_dn());
        const AdObject category_object = ad.search_object(category_dn, {ATTRIBUTE_LDAP_DISPLAY_NAME});
        const QString object_class = category_object.get_string(ATTRIBUTE_LDAP_DISPLAY_NAME);

        d->filter_containers.append(object_class);
    }

    // NOTE: domain not included for some reason, so add it manually
    d->filter_containers.append(CLASS_DOMAIN);

    // Make configuration and schema pass filter in dev mode so they are visible and can be fetched
    d->filter_containers.append({CLASS_CONFIGURATION, CLASS_dMD});
}

void AdConfig::load_permissionable_attributes(const QString &obj_class, AdInterface &ad) {
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_LDAP_DISPLAY_NAME, obj_class);
    QHash<QString, AdObject> results = ad.search(schema_dn(), SearchScope_Children, filter,
                                             {ATTRIBUTE_ALLOWED_ATTRIBUTES, ATTRIBUTE_SYSTEM_MAY_CONTAIN});
    if (results.isEmpty()) {
        return;
    }
    const AdObject class_schema_object = results.values()[0];

    const QStringList allowed_attrs = bytearray_list_to_string_list(class_schema_object.get_values(ATTRIBUTE_ALLOWED_ATTRIBUTES));
    const QStringList attrs_system_may_contain = bytearray_list_to_string_list(class_schema_object.get_values(ATTRIBUTE_SYSTEM_MAY_CONTAIN));

    // Use set to remove duplicates (just in case)
    QSet<QString> allowed_attrs_set = QSet<QString>(allowed_attrs.begin(), allowed_attrs.end());
    QSet<QString> may_contain_attrs_set = QSet<QString>(attrs_system_may_contain.begin(), attrs_system_may_contain.end());
    allowed_attrs_set |= may_contain_attrs_set;

    QSet<QString> permissionable_attrs_set = allowed_attrs_set;
    // Remove backlinks, constructed and system-only attributes
    for (const QString &attr : allowed_attrs_set) {
        if (get_attribute_is_backlink(attr) || get_attribute_is_constructed(attr) || get_attribute_is_system_only(attr)) {
            permissionable_attrs_set.remove(attr);
        }
    }

    QStringList permissionable_attrs = QStringList(permissionable_attrs_set.begin(), permissionable_attrs_set.end());
    permissionable_attrs.sort();
    d->class_permissionable_attributes_map[obj_class] = permissionable_attrs;

    for (const QString &inferior : d->class_possible_inferiors_map.value(obj_class, QStringList())) {
        if (inferior == obj_class || d->class_permissionable_attributes_map.contains(inferior)) {
            continue;
        }
        load_permissionable_attributes(inferior, ad);
    }
}

QList<QString> AdConfigPrivate::add_auxiliary_classes(const QList<QString> &object_classes) const {
    QList<QString> out;

    out += object_classes;

    for (const auto &object_class : object_classes) {
        const AdObject schema = class_schemas[object_class];
        out += schema.get_strings(ATTRIBUTE_AUXILIARY_CLASS);
        out += schema.get_strings(ATTRIBUTE_SYSTEM_AUXILIARY_CLASS);
    }

    out.removeDuplicates();

    return out;
}
