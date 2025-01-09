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

#ifndef AD_DEFINES_H
#define AD_DEFINES_H

/**
 * AD related enums and constants.
 */

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

enum AccountOption {
    AccountOption_Disabled,
    AccountOption_CantChangePassword,
    AccountOption_AllowReversibleEncryption,
    AccountOption_PasswordExpired,
    AccountOption_DontExpirePassword,
    AccountOption_UseDesKey,
    AccountOption_SmartcardRequired,
    AccountOption_CantDelegate,
    AccountOption_DontRequirePreauth,
    AccountOption_TrustedForDelegation,
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
    SystemFlagsBit_DomainCannotMove = 0x04000000,
    SystemFlagsBit_DomainCannotRename = 0x08000000,
    SystemFlagsBit_CannotDelete = 0x80000000
};

#define ROOT_DSE ""

#define ATTRIBUTE_CN "cn"
#define ATTRIBUTE_USER_ACCOUNT_CONTROL "userAccountControl"
#define ATTRIBUTE_LOCKOUT_TIME "lockoutTime"
#define ATTRIBUTE_ACCOUNT_EXPIRES "accountExpires"
#define ATTRIBUTE_PWD_LAST_SET "pwdLastSet"
#define ATTRIBUTE_NAME "name"
#define ATTRIBUTE_INITIALS "initials"
#define ATTRIBUTE_SAM_ACCOUNT_NAME "sAMAccountName"
#define ATTRIBUTE_SAM_ACCOUNT_TYPE "sAMAccountType"
#define ATTRIBUTE_DISPLAY_NAME "displayName"
#define ATTRIBUTE_DESCRIPTION "description"
#define ATTRIBUTE_USER_PRINCIPAL_NAME "userPrincipalName"
#define ATTRIBUTE_MAIL "mail"
#define ATTRIBUTE_OFFICE "physicalDeliveryOfficeName"
#define ATTRIBUTE_TELEPHONE_NUMBER "telephoneNumber"
#define ATTRIBUTE_TELEPHONE_NUMBER_OTHER "otherTelephone"
#define ATTRIBUTE_WWW_HOMEPAGE "wWWHomePage"
#define ATTRIBUTE_WWW_HOMEPAGE_OTHER "url"
#define ATTRIBUTE_COUNTRY_ABBREVIATION "c"
#define ATTRIBUTE_COUNTRY "co"
#define ATTRIBUTE_COUNTRY_CODE "countryCode"
#define ATTRIBUTE_CITY "l"
#define ATTRIBUTE_PO_BOX "postOfficeBox"
#define ATTRIBUTE_POSTAL_CODE "postalCode"
#define ATTRIBUTE_STATE "st"
#define ATTRIBUTE_STREET "streetAddress"
#define ATTRIBUTE_STREET_OU "street"
#define ATTRIBUTE_DN "distinguishedName"
#define ATTRIBUTE_OBJECT_CLASS "objectClass"
#define ATTRIBUTE_WHEN_CREATED "whenCreated"
#define ATTRIBUTE_WHEN_CHANGED "whenChanged"
#define ATTRIBUTE_USN_CHANGED "uSNChanged"
#define ATTRIBUTE_USN_CREATED "uSNCreated"
#define ATTRIBUTE_OBJECT_CATEGORY "objectCategory"
#define ATTRIBUTE_MEMBER "member"
#define ATTRIBUTE_MEMBER_OF "memberOf"
#define ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY "showInAdvancedViewOnly"
#define ATTRIBUTE_GROUP_TYPE "groupType"
#define ATTRIBUTE_FIRST_NAME "givenName"
#define ATTRIBUTE_LAST_NAME "sn"
#define ATTRIBUTE_DNS_HOST_NAME "dNSHostName"
#define ATTRIBUTE_INFO "info"
#define ATTRIBUTE_PASSWORD "unicodePwd"
#define ATTRIBUTE_GPLINK "gPLink"
#define ATTRIBUTE_GPOPTIONS "gPOptions"
#define ATTRIBUTE_DEPARTMENT "department"
#define ATTRIBUTE_COMPANY "company"
#define ATTRIBUTE_TITLE "title"
#define ATTRIBUTE_LAST_LOGON "lastLogon"
#define ATTRIBUTE_LAST_LOGON_TIMESTAMP "lastLogonTimestamp"
#define ATTRIBUTE_PWD_LAST_SET "pwdLastSet"
#define ATTRIBUTE_BAD_PWD_TIME "badPasswordTime"
#define ATTRIBUTE_OBJECT_SID "objectSid"
#define ATTRIBUTE_SYSTEM_FLAGS "systemFlags"
#define ATTRIBUTE_MAX_PWD_AGE "maxPwdAge"
#define ATTRIBUTE_MIN_PWD_AGE "minPwdAge"
#define ATTRIBUTE_LOCKOUT_DURATION "lockoutDuration"
#define ATTRIBUTE_IS_CRITICAL_SYSTEM_OBJECT "isCriticalSystemObject"
#define ATTRIBUTE_GPC_FILE_SYS_PATH "gPCFileSysPath"
#define ATTRIBUTE_GPC_FUNCTIONALITY_VERSION "gpCFunctionalityVersion"
#define ATTRIBUTE_VERSION_NUMBER "versionNumber"
#define ATTRIBUTE_FLAGS "flags"
#define ATTRIBUTE_OBJECT_GUID "objectGUID"
#define ATTRIBUTE_PRIMARY_GROUP_ID "primaryGroupID"
#define ATTRIBUTE_MANAGER "manager"
#define ATTRIBUTE_MANAGED_BY "managedBy"
#define ATTRIBUTE_DIRECT_REPORTS "directReports"
#define ATTRIBUTE_PROFILE_PATH "profilePath"
#define ATTRIBUTE_SCRIPT_PATH "scriptPath"
#define ATTRIBUTE_HOME_DIRECTORY "homeDirectory"
#define ATTRIBUTE_LAPS_PASSWORD "ms-Mcs-AdmPwd"
#define ATTRIBUTE_LAPS_EXPIRATION "ms-Mcs-AdmPwdExpirationTime"
#define ATTRIBUTE_HOME_PHONE "homePhone"
#define ATTRIBUTE_OTHER_HOME_PHONE "otherHomePhone"
#define ATTRIBUTE_PAGER "pager"
#define ATTRIBUTE_OTHER_PAGER "otherPager"
#define ATTRIBUTE_MOBILE "mobile"
#define ATTRIBUTE_OTHER_MOBILE "otherMobile"
#define ATTRIBUTE_FAX_NUMBER "facsimileTelephoneNumber"
#define ATTRIBUTE_OTHER_FAX_NUMBER "otherFacsimileTelephoneNumber"
#define ATTRIBUTE_IP_PHONE "ipPhone"
#define ATTRIBUTE_OTHER_IP_PHONE "otherIpPhone"
#define ATTRIBUTE_UPN_SUFFIXES "uPNSuffixes"
#define ATTRIBUTE_SECURITY_DESCRIPTOR "nTSecurityDescriptor"
#define ATTRIBUTE_RIGHTS_GUID "rightsGuid"
#define ATTRIBUTE_LOCATION "location"
#define ATTRIBUTE_OS "operatingSystem"
#define ATTRIBUTE_OS_VERSION "operatingSystemVersion"
#define ATTRIBUTE_OS_SERVICE_PACK "operatingSystemServicePack"
#define ATTRIBUTE_LOGON_HOURS "logonHours"
#define ATTRIBUTE_USER_WORKSTATIONS "userWorkstations"
#define ATTRIBUTE_VERSION_NUMBER "versionNumber"
#define ATTRIBUTE_SUPPORTED_CONTROL "supportedControl"
#define ATTRIBUTE_DS_SERVICE_NAME "dsServiceName"
#define ATTRIBUTE_SCHEMA_NAMING_CONTEXT "schemaNamingContext"
#define ATTRIBUTE_CONFIGURATION_NAMING_CONTEXT "configurationNamingContext"
#define ATTRIBUTE_ROOT_DOMAIN_NAMING_CONTEXT "rootDomainNamingContext"
#define ATTRIBUTE_FSMO_ROLE_OWNER "fSMORoleOwner"
#define ATTRIBUTE_SERVER_NAME "serverName"
#define ATTRIBUTE_SCHEMA_ID_GUID "schemaIDGUID"
#define ATTRIBUTE_APPLIES_TO "appliesTo"
#define ATTRIBUTE_VALID_ACCESSES "validAccesses"
#define ATTRIBUTE_UNC_NAME "uNCName"
#define ATTRIBUTE_KEYWORDS "keywords"
#define ATTRIBUTE_CREATION_TIME "creationTime"
#define ATTRIBUTE_LOCKOUT_OBSERVATION_WINDOW "lockOutObservationWindow"
#define ATTRIBUTE_FORCE_LOGOFF "forceLogoff"
#define ATTRIBUTE_MS_DS_SUPPORTED_ETYPES "msDS-SupportedEncryptionTypes"
#define ATTRIBUTE_DOMAIN_FUNCTIONALITY_LEVEL "domainFunctionality"
#define ATTRIBUTE_FOREST_FUNCTIONALITY_LEVEL "forestFunctionality"
#define ATTRIBUTE_OBJECT_VERSION "objectVersion"
#define ATTRIBUTE_SERVER_REFERENCE "serverReference"
#define ATTRIBUTE_SERVER_REFERENCE_BL "serverReferenceBL"
#define ATTRIBUTE_MS_DS_LOCKOUT_DURATION "msDS-LockoutDuration"
#define ATTRIBUTE_MS_DS_LOCKOUT_OBSERVATION_WINDOW "msDS-LockoutObservationWindow"
#define ATTRIBUTE_MS_DS_MAX_PASSWORD_AGE "msDS-MaximumPasswordAge"
#define ATTRIBUTE_MS_DS_MIN_PASSWORD_AGE "msDS-MinimumPasswordAge"
#define ATTRIBUTE_MS_DS_LOCKOUT_THRESHOLD "msDS-LockoutThreshold"
#define ATTRIBUTE_MS_DS_MIN_PASSWORD_LENGTH "msDS-MinimumPasswordLength"
#define ATTRIBUTE_MS_DS_PASSWORD_COMPLEXITY_ENABLED "msDS-PasswordComplexityEnabled"
#define ATTRIBUTE_MS_DS_PASSWORD_HISTORY_LENGTH "msDS-PasswordHistoryLength"
#define ATTRIBUTE_MS_DS_PASSWORD_REVERSIBLE_ENCRYPTION_ENABLED "msDS-PasswordReversibleEncryptionEnabled"
#define ATTRIBUTE_MS_DS_PASSWORD_SETTINGS_PRECEDENCE "msDS-PasswordSettingsPrecedence"
#define ATTRIBUTE_PSO_APPLIES_TO "msDS-PSOAppliesTo"
#define ATTRIBUTE_DEFAULT_NAMING_CONTEXT "defaultNamingContext"
#define ATTRIBUTE_MSDS_USER_PASSWORD_EXPIRY_TIME_COMPUTED "msDS-UserPasswordExpiryTimeComputed"
#define ATTRIBUTE_MSDS_USER_ACCOUNT_CONTROL_COMPUTED "msDS-User-Account-Control-Computed"
#define ATTRIBUTE_MIDDLE_NAME "middleName"


#define CLASS_GROUP "group"
#define CLASS_USER "user"
#define CLASS_CONTAINER "container"
#define CLASS_OU "organizationalUnit"
#define CLASS_PERSON "person"
#define CLASS_GP_CONTAINER "groupPolicyContainer"
#define CLASS_DOMAIN "domainDNS"
#define CLASS_TOP "top"
#define CLASS_COMPUTER "computer"
#define CLASS_ORG_PERSON "organizationalPerson"
#define CLASS_CONTACT "contact"
#define CLASS_PRINTER "printQueue"
#define CLASS_DEFAULT "default"
#define CLASS_CONFIGURATION "configuration"
#define CLASS_TRUSTED_DOMAIN "trustedDomain"
#define CLASS_INET_ORG_PERSON "inetOrgPerson"
#define CLASS_FOREIGN_SECURITY_PRINCIPAL "foreignSecurityPrincipal"
#define CLASS_SHARED_FOLDER "volume"
#define CLASS_RPC_SERVICES "rpcContainer"
#define CLASS_CERTIFICATE_TEMPLATE "pKICertificateTemplate"
#define CLASS_MSMQ_GROUP "msMQ-Group"
#define CLASS_MSMQ_QUEUE_ALIAS "msMQ-Custom-Recipient"
#define CLASS_REMOTE_STORAGE_SERVICE "remoteStorageServicePoint"
#define CLASS_SITE "site"
#define CLASS_SERVER "server"
#define CLASS_SERVERS_CONTAINER "serversContainer"
#define CLASS_PSO_CONTAINER "msDS-PasswordSettingsContainer"
#define CLASS_PSO "msDS-PasswordSettings"
// NOTE: for schema object
#define CLASS_dMD "dMD"
#define CLASS_CONTROL_ACCESS_RIGHT "controlAccessRight"

#define OBJECT_CATEGORY_PERSON "Person"
#define OBJECT_CATEGORY_GROUP "Group"
#define OBJECT_CATEGORY_OU "Organizational-Unit"
#define OBJECT_CATEGORY_COMPUTER "Computer"
#define OBJECT_CATEGORY_DOMAIN_DNS "Domain-DNS"
#define OBJECT_CATEGORY_CONTAINER "Container"
#define OBJECT_CATEGORY_GP_CONTAINER "Group-Policy-Container"
#define OBJECT_CATEGORY_VOLUME "Volume"
#define OBJECT_CATEGORY_BUILTIN "Builtin-Domain"
#define OBJECT_CATEGORY_LOST_AND_FOUND "Lost_And_Found"
#define OBJECT_CATEGORY_INFRASTRUCTURE_UPDATE "Infrastructure-Update"
#define OBJECT_CATEGORY_MSDS_QUOTA_CONTAINER "ms-DS-Quota-Container"
#define OBJECT_CATEGORY_SERVERS_CONTAINER "Servers-Container"
#define OBJECT_CATEGORY_SITE "Site"
#define OBJECT_CATEGORY_PSO_CONTAINER "ms-DS-Password-Settings-Container"
#define OBJECT_CATEGORY_PSO "ms-DS-Password-Settings"

// These categories are not AD object categories. They are used within ADMC context
#define ADMC_CATEGORY_QUERY_ITEM "query-item"
#define ADMC_CATEGORY_QUERY_FOLDER "query-folder"
#define ADMC_CATEGORY_FSMO_ROLE "FSMO-role"
#define ADMC_CATEGORY_FSMO_ROLE_CONTAINER "FSMO-role-container"
#define ADMC_CATEGORY_GP_OBJECTS "group-policy-objects"
#define ADMC_CATEGORY_ALL_POLICIES_FOLDER "all-policies-folder"
#define ADMC_CATEGORY_DOMAIN_INFO_ITEM "domain-info-item"

#define LOCKOUT_UNLOCKED_VALUE "0"

#define AD_LARGE_INTEGER_DATETIME_NEVER_1 "0"
#define AD_LARGE_INTEGER_DATETIME_NEVER_2 "9223372036854775807"

#define AD_PWD_LAST_SET_EXPIRED "0"
#define AD_PWD_LAST_SET_RESET "-1"

#define LDAP_BOOL_TRUE "TRUE"
#define LDAP_BOOL_FALSE "FALSE"

#define GPOPTIONS_INHERIT "0"
#define GPOPTIONS_BLOCK_INHERITANCE "1"

#define GROUP_TYPE_BIT_SECURITY 0x80000000
#define GROUP_TYPE_BIT_SYSTEM 0x00000001

#define DATETIME_DISPLAY_FORMAT "dd.MM.yy hh:mm UTCt"

const long long MILLIS_TO_100_NANOS = 10000LL;

#define MATCHING_RULE_IN_CHAIN_OID "1.2.840.113556.1.4.1941"

#define LDAP_SERVER_SD_FLAGS_OID "1.2.840.113556.1.4.801"
#define OWNER_SECURITY_INFORMATION 0x01
#define GROUP_SECURITY_INFORMATION 0x02
#define SACL_SECURITY_INFORMATION 0x08
#define DACL_SECURITY_INFORMATION 0x04

#define SAM_NAME_BAD_CHARS "@\"[]:;|=+*?<>/\\,"
#define UPN_BAD_CHARS "#,+\"\\<>"
// NOTE: names technically can contain these chars but
// we choose to be more strict about it
#define NAME_BAD_CHARS ",\\#+<>;\"="

#define UAC_SCRIPT 0x00000001
#define UAC_ACCOUNTDISABLE 0x00000002
#define UAC_HOMEDIR_REQUIRED 0x00000008
#define UAC_LOCKOUT 0x00000010
#define UAC_PASSWD_NOTREQD 0x00000020
#define UAC_PASSWD_CANT_CHANGE 0x00000040
#define UAC_ENCRYPTED_TEXT_PASSWORD_ALLOWED 0x00000080
#define UAC_TEMP_DUPLICATE_ACCOUNT 0x00000100
#define UAC_NORMAL_ACCOUNT 0x00000200
#define UAC_INTERDOMAIN_TRUST_ACCOUNT 0x00000800
#define UAC_WORKSTATION_TRUST_ACCOUNT 0x00001000
#define UAC_SERVER_TRUST_ACCOUNT 0x00002000
#define UAC_DONT_EXPIRE_PASSWORD 0x00010000
#define UAC_MNS_LOGON_ACCOUNT 0x00020000
#define UAC_SMARTCARD_REQUIRED 0x00040000
#define UAC_TRUSTED_FOR_DELEGATION 0x00080000
#define UAC_NOT_DELEGATED 0x00100000
#define UAC_USE_DES_KEY_ONLY 0x00200000
#define UAC_DONT_REQUIRE_PREAUTH 0x00400000
#define UAC_ERROR_PASSWORD_EXPIRED 0x00800000
#define UAC_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION 0x01000000
#define UAC_PARTIAL_SECRETS_ACCOUNT 0x04000000
#define UAC_USER_USE_AES_KEYS 0x08000000

#define SAM_DOMAIN_OBJECT 0x0
#define SAM_GROUP_OBJECT 0x10000000
#define SAM_NON_SECURITY_GROUP_OBJECT 0x10000001
#define SAM_ALIAS_OBJECT 0x20000000
#define SAM_NON_SECURITY_ALIAS_OBJECT 0x20000001
#define SAM_USER_OBJECT 0x30000000
#define SAM_NORMAL_USER_ACCOUNT 0x30000000
#define SAM_MACHINE_ACCOUNT 0x30000001
#define SAM_TRUST_ACCOUNT 0x30000002
#define SAM_APP_BASIC_GROUP 0x40000000
#define SAM_APP_QUERY_GROUP 0x40000001
#define SAM_ACCOUNT_TYPE_MAX 0x7fffffff

// NOTE: this is according to Microsoft, haven't found
// any better source
#define SAM_NAME_MAX_LENGTH 20
#define SAM_NAME_COMPUTER_MAX_LENGTH 16

// NOTE: define supported encryption types bit flags
// for msDs-SupportedEncryptionTypes attribute
#define ETYPES_DES_CBC_CRC 0x00000001
#define ETYPES_DES_CBC_MD5 0x00000002
#define ETYPES_RC4_HMAC_MD5 0x00000004
#define ETYPES_AES128_CTS_HMAC_SHA1_96 0x00000008
#define ETYPES_AES256_CTS_HMAC_SHA1_96 0x00000010

enum SearchScope {
    SearchScope_Object,
    SearchScope_Children,
    // NOTE: Descendants scope appears to not work, from the
    // ldap_search manual: "Note that the latter requires
    // the server support the LDAP Subordinates Search Scope
    // extension."
    SearchScope_Descendants,
    SearchScope_All,
};

enum CertStrategy {
    CertStrategy_Never,
    CertStrategy_Hard,
    CertStrategy_Demand,
    CertStrategy_Allow,
    CertStrategy_Try,
};

#endif /* AD_DEFINES_H */
