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

#include "ad_enums.h"
#include "ad_object.h"

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
#define ATTRIBUTE_TELEPHONE_NUMBER_OTHER "otherTelephone"
#define ATTRIBUTE_WWW_HOMEPAGE          "wWWHomePage"
#define ATTRIBUTE_WWW_HOMEPAGE_OTHER    "url"
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
#define ATTRIBUTE_MAX_PWD_AGE           "maxPwdAge"
#define ATTRIBUTE_MIN_PWD_AGE           "minPwdAge"
#define ATTRIBUTE_LOCKOUT_DURATION      "lockoutDuration"
#define ATTRIBUTE_IS_CRITICAL_SYSTEM_OBJECT "isCriticalSystemObject"
#define ATTRIBUTE_GPC_FILE_SYS_PATH     "gPCFileSysPath"
#define ATTRIBUTE_GPC_FUNCTIONALITY_VERSION "gpCFunctionalityVersion"
#define ATTRIBUTE_VERSION_NUMBER        "versionNumber"
#define ATTRIBUTE_FLAGS                 "flags"
#define ATTRIBUTE_OBJECT_GUID           "objectGUID"
#define ATTRIBUTE_PRIMARY_GROUP_ID      "primaryGroupID"
#define ATTRIBUTE_MANAGER               "manager"
#define ATTRIBUTE_DIRECT_REPORTS        "directReports"

#define CLASS_GROUP                     "group"
#define CLASS_USER                      "user"
#define CLASS_CONTAINER                 "container"
#define CLASS_OU                        "organizationalUnit"
#define CLASS_PERSON                    "person"
#define CLASS_GP_CONTAINER              "groupPolicyContainer"
#define CLASS_DOMAIN                    "domainDNS"
#define CLASS_TOP                       "top"
#define CLASS_COMPUTER                  "computer"
#define CLASS_ORG_PERSON                "organizationalPerson"
#define CLASS_CONTACT                   "contact"
// TODO: not sure if this is correct class for printer, but display name for this in display specifiers is "Printer"
#define CLASS_PRINTER                   "printQueue"
#define CLASS_DEFAULT                   "default"
#define CLASS_CONFIGURATION             "configuration"
#define CLASS_TRUSTED_DOMAIN            "trustedDomain"
#define CLASS_INET_ORG_PERSON           "inetOrgPerson"
#define CLASS_FOREIGN_SECURITY_PRINCIPAL "foreignSecurityPrincipal"
#define CLASS_SHARED_FOLDER             "volume"
#define CLASS_RPC_SERVICES              "rpcContainer"
#define CLASS_CERTIFICATE_TEMPLATE      "pKICertificateTemplate"
#define CLASS_MSMQ_GROUP                "msMQ-Group"
#define CLASS_MSMQ_QUEUE_ALIAS          "msMQ-Custom-Recipient"
#define CLASS_REMOTE_STORAGE_SERVICE    "remoteStorageServicePoint"
// NOTE: for schema object
#define CLASS_dMD                       "dMD"

#define LOCKOUT_UNLOCKED_VALUE "0"

#define AD_LARGE_INTEGER_DATETIME_NEVER_1 "0"
#define AD_LARGE_INTEGER_DATETIME_NEVER_2 "9223372036854775807"

#define AD_PWD_LAST_SET_EXPIRED "0"
#define AD_PWD_LAST_SET_RESET   "-1"

#define LDAP_BOOL_TRUE  "TRUE"
#define LDAP_BOOL_FALSE "FALSE"

#define GPOPTIONS_INHERIT           "0"
#define GPOPTIONS_BLOCK_INHERITANCE "1"

#define GROUP_TYPE_BIT_SECURITY 0x80000000
#define GROUP_TYPE_BIT_SYSTEM 0x00000001

#define SEARCH_ALL_ATTRIBUTES "SEARCH_ALL_ATTRIBUTES"

enum SearchScope {
    SearchScope_Object,
    SearchScope_Children,
    SearchScope_Descendants,
    SearchScope_All,
};

const qint64 MILLIS_TO_100_NANOS = 10000LL;
const qint64 SECONDS_TO_MILLIS   = 1000LL;
const qint64 MINUTES_TO_SECONDS  = 60LL;
const qint64 HOURS_TO_SECONDS    = MINUTES_TO_SECONDS * 60LL;
const qint64 DAYS_TO_SECONDS     = HOURS_TO_SECONDS * 24LL;

typedef struct ldap LDAP;
typedef struct _SMBCCTX SMBCCTX;
class AdConfig;

class AdInterface final : public QObject {
Q_OBJECT

private:
    enum DoStatusMsg {
        DoStatusMsg_Yes,
        DoStatusMsg_No
    };

public:
    static AdInterface *instance();

    bool login(const QString &domain, const QString &site);

    void refresh();

    // Use this if you are doing a series of AD modifications.
    // During the batch, modified() signals won't be emitted
    // and once the batch is complete one modified() signal
    // is emitted, so that GUI is reloaded only once.
    void start_batch();
    void end_batch();

    AdConfig *config() const;
    QString domain() const;
    QString search_base() const;
    QString host() const;
    QString configuration_dn() const;
    QString schema_dn() const;

    // NOTE: search f-ns need to communicate with the AD server, 
    // so use them only for infrequent calls. Also try to ask
    // only for attributes that you need.
    QHash<QString, AdObject> search(const QString &filter, const QList<QString> &attributes, const SearchScope scope_enum, const QString &custom_search_base = QString());
    AdObject search_object(const QString &dn, const QList<QString> &attributes = {SEARCH_ALL_ATTRIBUTES});

    bool attribute_add(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace_values(const QString &dn, const QString &attribute, const QList<QByteArray> &values, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_delete_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    
    bool attribute_add_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_delete_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);

    bool attribute_replace_int(const QString &dn, const QString &attribute, const int value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace_datetime(const QString &dn, const QString &attribute, const QDateTime &datetime);

    bool object_add(const QString &dn, const char **classes);
    bool object_delete(const QString &dn);
    bool object_move(const QString &dn, const QString &new_container);
    bool object_rename(const QString &dn, const QString &new_name);

    bool group_add_user(const QString &group_dn, const QString &user_dn);
    bool group_remove_user(const QString &group_dn, const QString &user_dn);
    bool group_set_primary_for_user(const QString &group_dn, const QString &user_dn);
    bool group_set_scope(const QString &dn, GroupScope scope);
    bool group_set_type(const QString &dn, GroupType type);

    bool user_set_pass(const QString &dn, const QString &password);
    bool user_set_account_option(const QString &dn, AccountOption option, bool set);
    bool user_unlock(const QString &dn);

    bool object_can_drop(const QString &dn, const QString &target_dn);
    void object_drop(const QString &dn, const QString &target_dn);

    bool create_gpo(const QString &name);
    bool delete_gpo(const QString &dn);

    void command(QStringList args);

signals:
    void modified();

private:
    LDAP *ld;
    SMBCCTX *smbc;
    AdConfig *m_config;
    QString m_domain;
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

    AdInterface(const AdInterface&) = delete;
    AdInterface& operator=(const AdInterface&) = delete;
    AdInterface(AdInterface&&) = delete;
    AdInterface& operator=(AdInterface&&) = delete;
};

AdInterface *AD();

bool large_integer_datetime_is_never(const QString &value);
QString datetime_qdatetime_to_string(const QString &attribute, const QDateTime &datetime);
QDateTime datetime_string_to_qdatetime(const QString &attribute, const QString &raw_value);

QString account_option_string(const AccountOption &option);
int account_option_bit(const AccountOption &option);

int group_scope_bit(GroupScope scope);
QString group_scope_string(GroupScope scope);

QString group_type_string(GroupType type);

QString attribute_display_value(const QString &attribute, const QByteArray &value);

QString sysvol_path_to_smb(const QString &sysvol_path);

QString extract_rid_from_sid(const QByteArray &sid);

#endif /* AD_INTERFACE_H */
