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

#ifndef AD_INTERFACE_H
#define AD_INTERFACE_H

#include "ad_interface_defines.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QMap>
#include <QDateTime>
#include <QSet>
#include <QIcon>

// Interface to AD/LDAP
// Emits modified() signal after performing an action that
// modifies data on the AD server
// Reload GUI state when that signal is emitted

#define ATTRIBUTE_USER_ACCOUNT_CONTROL  "userAccountControl"
#define ATTRIBUTE_LOCKOUT_TIME          "lockoutTime"
#define ATTRIBUTE_ACCOUNT_EXPIRES       "accountExpires"
#define ATTRIBUTE_PWD_LAST_SET          "pwdLastSet"
#define ATTRIBUTE_NAME                  "name"
#define ATTRIBUTE_CN                    "cn"
#define ATTRIBUTE_INITIALS              "initials"
#define ATTRIBUTE_SAMACCOUNT_NAME       "sAMAccountName"
#define ATTRIBUTE_DISPLAY_NAME          "displayName"
#define ATTRIBUTE_DESCRIPTION           "description"
#define ATTRIBUTE_USER_PRINCIPAL_NAME   "userPrincipalName"
#define ATTRIBUTE_MAIL                  "mail"
#define ATTRIBUTE_OFFICE                "physicalDeliveryOfficeName"
#define ATTRIBUTE_TELEPHONE_NUMBER      "telephoneNumber"
#define ATTRIBUTE_WWW_HOMEPAGE          "wWWHomePage"
#define ATTRIBUTE_COUNTRY_ABBREVIATION  "c"
#define ATTRIBUTE_COUNTRY               "co"
#define ATTRIBUTE_COUNTRY_CODE          "countryCode"
#define ATTRIBUTE_CITY                  "l"
#define ATTRIBUTE_PO_BOX                "postOfficeBox"
#define ATTRIBUTE_POSTAL_CODE           "postalCode"
#define ATTRIBUTE_STATE                 "st"
#define ATTRIBUTE_STREET                "streetAddress"
#define ATTRIBUTE_DISTINGUISHED_NAME    "distinguishedName"
#define ATTRIBUTE_OBJECT_CLASS          "objectClass"
#define ATTRIBUTE_WHEN_CREATED          "whenCreated"
#define ATTRIBUTE_WHEN_CHANGED          "whenChanged"
#define ATTRIBUTE_USN_CHANGED           "uSNChanged"
#define ATTRIBUTE_USN_CREATED           "uSNCreated"
#define ATTRIBUTE_OBJECT_CATEGORY       "objectCategory"
#define ATTRIBUTE_MEMBER                "member"
#define ATTRIBUTE_MEMBER_OF             "memberOf"
#define ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY "showInAdvancedViewOnly"
#define ATTRIBUTE_GROUP_TYPE            "groupType"
#define ATTRIBUTE_FIRST_NAME            "givenName"
#define ATTRIBUTE_LAST_NAME             "sn"
#define ATTRIBUTE_DNS_HOST_NAME         "dNSHostName"
#define ATTRIBUTE_INFO                  "info"
#define ATTRIBUTE_PASSWORD              "unicodePwd"
#define ATTRIBUTE_GPLINK                "gPLink"
#define ATTRIBUTE_GPOPTIONS             "gPOptions"
#define ATTRIBUTE_DEPARTMENT            "department"
#define ATTRIBUTE_COMPANY               "company"
#define ATTRIBUTE_TITLE                 "title"
#define ATTRIBUTE_LAST_LOGON            "lastLogon"
#define ATTRIBUTE_LAST_LOGON_TIMESTAMP  "lastLogonTimestamp"
#define ATTRIBUTE_PWD_LAST_SET          "pwdLastSet"
#define ATTRIBUTE_LOCKOUT_TIME          "lockoutTime"
#define ATTRIBUTE_BAD_PWD_TIME          "badPasswordTime"
#define ATTRIBUTE_OBJECT_SID            "objectSid"
#define ATTRIBUTE_SYSTEM_FLAGS          "systemFlags"

#define CLASS_GROUP                     "group"
#define CLASS_USER                      "user"
#define CLASS_CONTAINER                 "container"
#define CLASS_OU                        "organizationalUnit"
#define CLASS_BUILTIN_DOMAIN            "builtinDomain"
#define CLASS_PERSON                    "person"
#define CLASS_GP_CONTAINER              "groupPolicyContainer"
#define CLASS_DOMAIN                    "domain"
#define CLASS_TOP                       "top"
#define CLASS_COMPUTER                  "computer"
#define CLASS_ORG_PERSON                "organizationalPerson"
#define CLASS_DEFAULT                   "default"
#define CLASS_CONFIGURATION             "configuration"
// NOTE: for schema object
#define CLASS_dMD                       "dMD"

#define LOCKOUT_UNLOCKED_VALUE "0"

#define AD_LARGEINTEGERTIME_NEVER_1 "0"
#define AD_LARGEINTEGERTIME_NEVER_2 "9223372036854775807"

#define LDAP_BOOL_TRUE  "TRUE"
#define LDAP_BOOL_FALSE "FALSE"

#define GPOPTIONS_INHERIT           "0"
#define GPOPTIONS_BLOCK_INHERITANCE "1"

enum AccountOption {
    AccountOption_Disabled,
    AccountOption_PasswordExpired,
    AccountOption_DontExpirePassword,
    AccountOption_UseDesKey,
    AccountOption_SmartcardRequired,
    AccountOption_CantDelegate,
    AccountOption_DontRequirePreauth,
    AccountOption_COUNT
};

enum GroupScope {
    GroupScope_Global,
    GroupScope_DomainLocal,
    GroupScope_Universal,
    GroupScope_COUNT
};

enum GroupType {
    GroupType_Security,
    GroupType_Distribution,
    GroupType_COUNT
};

enum SystemFlagsBit {
    SystemFlagsBit_CannotMove = 0x04000000,
    SystemFlagsBit_CannotRename = 0x08000000,
    SystemFlagsBit_CannotDelete = 0x80000000
};

enum SearchScope {
    SearchScope_Object,
    SearchScope_Children,
    SearchScope_Descendants,
    SearchScope_All,
};

typedef struct ldap LDAP;

enum DoStatusMsg {
    DoStatusMsg_Yes,
    DoStatusMsg_No
};

class AdInterface final : public QObject {
Q_OBJECT

public:
    static AdInterface *instance();

    bool login(const QString &host_arg, const QString &domain);

    // Use this if you are doing a series of AD modifications.
    // During the batch, modified() signals won't be emitted.
    // Once the batch is complete one modified() signal
    // is emitted, so that GUI is reloaded only once.
    void start_batch();
    void end_batch();

    QString search_base() const;
    QString host() const;
    QString configuration_dn() const;
    QString schema_dn() const;

    QHash<QString, Attributes> search(const QString &filter, const QList<QString> &attributes, const SearchScope scope_enum, const QString &custom_search_base = QString());
    QList<QString> search_dns(const QString &filter, const QString &custom_search_base = QString());

    // NOTE: all request f-ns make an LDAP request, avoid using them unless it's an infrequent call 
    Attributes attribute_request_all(const QString &dn);
    QList<QByteArray> attribute_request_values(const QString &dn, const QString &attribute);
    QByteArray attribute_request_value(const QString &dn, const QString &attribute);

    QList<QString> attribute_request_strings(const QString &dn, const QString &attribute);

    bool attribute_add(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_delete(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    
    bool attribute_add_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_delete_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);

    bool attribute_replace_int(const QString &dn, const QString &attribute, const int value);
    bool attribute_replace_datetime(const QString &dn, const QString &attribute, const QDateTime &datetime);

    bool object_add(const QString &dn, const char **classes);
    bool object_delete(const QString &dn);
    bool object_move(const QString &dn, const QString &new_container);
    bool object_rename(const QString &dn, const QString &new_name);

    bool user_set_pass(const QString &dn, const QString &password);
    bool user_set_account_option(const QString &dn, AccountOption option, bool set);
    bool user_unlock(const QString &dn);
    
    bool group_add_user(const QString &group_dn, const QString &user_dn);
    bool group_remove_user(const QString &group_dn, const QString &user_dn);
    bool group_set_scope(const QString &dn, GroupScope scope);
    bool group_set_type(const QString &dn, GroupType type);

    bool object_can_drop(const QString &dn, const QString &target_dn);
    void object_drop(const QString &dn, const QString &target_dn);

    void command(QStringList args);

signals:
    void modified();

private:
    LDAP *ld;
    QString m_search_base;
    QString m_configuration_dn;
    QString m_schema_dn;
    QString m_host;

    bool suppress_not_found_error = false;
    bool batch_in_progress = false;
        
    AdInterface();

    void emit_modified();
    void success_status_message(const QString &msg, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    void error_status_message(const QString &context, const QString &error, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    QString default_error() const;

    bool get_systemflags_bit(const SystemFlagsBit bit);

    AdInterface(const AdInterface&) = delete;
    AdInterface& operator=(const AdInterface&) = delete;
    AdInterface(AdInterface&&) = delete;
    AdInterface& operator=(AdInterface&&) = delete;
}; 

QList<QString> get_domain_hosts(const QString &domain, const QString &site);
QString extract_name_from_dn(const QString &dn);
QString extract_parent_dn_from_dn(const QString &dn);
QString filter_EQUALS(const QString &attribute, const QString &value);
QString filter_AND(const QString &a, const QString &b);
QString filter_OR(const QString &a, const QString &b);
QString filter_NOT(const QString &a);
QString get_account_option_description(const AccountOption &option);
int get_account_option_bit(const AccountOption &option);
bool attribute_is_datetime(const QString &attribute);
bool datetime_is_never(const QString &attribute, const QString &value);
QString datetime_to_string(const QString &attribute, const QDateTime &datetime);
QDateTime datetime_raw_to_datetime(const QString &attribute, const QString &raw_value);
QString group_scope_to_string(GroupScope scope);
QString group_type_to_string(GroupType type);
QIcon get_object_icon(const Attributes &attributes);
QString attribute_binary_value_to_display_value(const QString &attribute, const QByteArray &value_bytes);
QString attribute_value_to_display_value(const QString &attribute, const QString &value);
QString object_sid_to_display_string(const QByteArray &bytes);

bool user_get_account_option(const Attributes &attributes, AccountOption option);
GroupScope group_get_scope(const Attributes &attributes);
GroupType group_get_type(const Attributes &attributes);

bool object_is_class(const Attributes &attributes, const QString &object_class);
bool object_is_user(const Attributes &attributes);
bool object_is_group(const Attributes &attributes);
bool object_is_container(const Attributes &attributes);
bool object_is_ou(const Attributes &attributes);
bool object_is_policy(const Attributes &attributes);
bool object_is_computer(const Attributes &attributes);

QList<QByteArray> attribute_get_values(const Attributes &attributes, const QString &attribute);
QByteArray attribute_get_value(const Attributes &attributes, const QString &attribute);
QList<QString> attribute_get_strings(const Attributes &attributes, const QString &attribute);
bool attribute_get_system_flag(const Attributes &attributes, const SystemFlagsBit bit);
int attribute_get_int(const Attributes &attributes, const QString &attribute);

#endif /* AD_INTERFACE_H */
