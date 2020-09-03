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

#include <QObject>
#include <QList>
#include <QString>
#include <QMap>
#include <QHash>
#include <QDateTime>
#include <QSet>

// Interface between the GUI and AdConnection
// Stores attributes cache of objects
// Attributes cache is expanded as more objects are loaded and
// is updated on object changes
// Emits various signals for AD operation successes/failures

// TODO: not sure if AD datetime fromat is always this one, LDAP allows multiple alternatives: https://ldapwiki.com/wiki/DateTime
#define ISO8601_FORMAT_STRING "yyyyMMddhhmmss.zZ"

#define ATTRIBUTE_USER_ACCOUNT_CONTROL  "userAccountControl"
#define ATTRIBUTE_LOCKOUT_TIME          "lockoutTime"
#define ATTRIBUTE_ACCOUNT_EXPIRES       "accountExpires"
#define ATTRIBUTE_PWD_LAST_SET          "pwdLastSet"
#define ATTRIBUTE_NAME                  "name"
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
#define ATTRIBUTE_CN                    "cn"
#define ATTRIBUTE_DISTINGUISHED_NAME    "distinguishedName"
#define ATTRIBUTE_OBJECT_CLASS          "objectClass"
#define ATTRIBUTE_WHEN_CREATED          "whenCreated"
#define ATTRIBUTE_WHEN_CHANGED          "whenChanged"
#define ATTRIBUTE_USN_CHANGED           "uSNChanged"
#define ATTRIBUTE_USN_CREATED           "uSNCreated"
#define ATTRIBUTE_OBJECT_CATEGORY       "objectCategory"
#define ATTRIBUTE_MEMBER                "member"
#define ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY "showInAdvancedViewOnly"
#define ATTRIBUTE_GROUP_TYPE            "groupType"
#define ATTRIBUTE_FIRST_NAME            "givenName"
#define ATTRIBUTE_LAST_NAME             "sn"
#define ATTRIBUTE_DNS_HOST_NAME         "dNSHostName"
#define ATTRIBUTE_INFO                  "info"
#define ATTRIBUTE_PASSWORD              "unicodePwd"

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

#define LOCKOUT_UNLOCKED_VALUE "0"

#define AD_LARGEINTEGERTIME_NEVER_1 "0"
#define AD_LARGEINTEGERTIME_NEVER_2 "9223372036854775807"

#define LDAP_BOOL_TRUE  "TRUE"
#define LDAP_BOOL_FALSE "FALSE"

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

typedef QMap<QString, QList<QString>> Attributes;
typedef struct ldap LDAP;

class AdInterface final : public QObject {
Q_OBJECT

public:
    AdInterface(const AdInterface&) = delete;
    AdInterface& operator=(const AdInterface&) = delete;
    AdInterface(AdInterface&&) = delete;
    AdInterface& operator=(AdInterface&&) = delete;
    
    static AdInterface *instance();

    static QList<QString> get_domain_hosts(const QString &domain, const QString &site);

    bool login(const QString &host, const QString &domain);

    // Use this if you are doing a series of AD modifications
    // During a batch, cache won't update and so UI won't reload
    // At the end of the batch cache updates once and UI reloads once
    // Each start call must be followed by a matching end call!
    void start_batch();
    void end_batch();
    bool batch_is_in_progress() const;

    QString get_search_base() const;

    QList<QString> list(const QString &dn);
    QList<QString> search(const QString &filter);
    
    Attributes get_all_attributes(const QString &dn);
    QList<QString> attribute_get_multi(const QString &dn, const QString &attribute);
    QString attribute_get(const QString &dn, const QString &attribute);

    bool attribute_bool_get(const QString &dn, const QString &attribute);
    bool attribute_bool_replace(const QString &dn, const QString &attribute, bool value);

    int attribute_int_get(const QString &dn, const QString &attribute);
    bool attribute_int_replace(const QString &dn, const QString &attribute, const int value);

    bool attribute_add(const QString &dn, const QString &attribute, const QString &value);
    bool attribute_replace(const QString &dn, const QString &attribute, const QString &value);
    bool attribute_delete(const QString &dn, const QString &attribute, const QString &value);
    bool object_add(const QString &dn, const char **classes);
    bool object_delete(const QString &dn);
    bool object_move(const QString &dn, const QString &new_container);
    bool object_rename(const QString &dn, const QString &new_name);
    bool user_set_pass(const QString &dn, const QString &password);
    bool user_set_account_option(const QString &dn, AccountOption option, bool set);
    bool user_unlock(const QString &dn);
    void update_cache(const QList<QString> &changed_dns);
    
    QDateTime attribute_datetime_get(const QString &dn, const QString &attribute);
    bool attribute_datetime_replace(const QString &dn, const QString &attribute, const QDateTime &datetime);

    bool group_add_user(const QString &group_dn, const QString &user_dn);
    bool group_remove_user(const QString &group_dn, const QString &user_dn);
    GroupScope group_get_scope(const QString &dn);
    bool group_set_scope(const QString &dn, GroupScope scope);
    GroupType group_get_type(const QString &dn);
    bool group_set_type(const QString &dn, GroupType type);
    bool group_is_system(const QString &dn);

    bool has_attributes(const QString &dn);
    bool is_class(const QString &dn, const QString &object_class);
    bool is_user(const QString &dn);
    bool is_group(const QString &dn);
    bool is_container(const QString &dn);
    bool is_ou(const QString &dn);
    bool is_policy(const QString &dn);
    bool is_container_like(const QString &dn);
    bool is_computer(const QString &dn);

    bool user_get_account_option(const QString &dn, AccountOption option);

    bool object_can_drop(const QString &dn, const QString &target_dn);
    void object_drop(const QString &dn, const QString &target_dn);

    void command(QStringList args);

signals:
    void modified();
    void logged_in();

private:
    LDAP *ld;
    QString search_base;

    QHash<QString, Attributes> attributes_cache;
    bool suppress_not_found_error = false;
    QSet<QString> batched_dns;
    bool batch_in_progress = false;
        
    AdInterface();

    bool should_emit_status_message(int result);
    void success_status_message(const QString &msg);
    void error_status_message(const QString &context, const QString &error);
    QString default_error(int ad_result) const;
}; 

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
QString datetime_raw_to_display_string(const QString &attribute, const QString &raw_value);
QDateTime datetime_raw_to_datetime(const QString &attribute, const QString &raw_value);
QString group_scope_to_string(GroupScope scope);
QString group_type_to_string(GroupType type);
QString object_class_display_string(const QString &c);

#endif /* AD_INTERFACE_H */
