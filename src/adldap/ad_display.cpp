/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "ad_display.h"

#include "ad_config.h"
#include "ad_defines.h"
#include "ad_utils.h"
#include "samba/dom_sid.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDateTime>
#include <QList>
#include <QString>
#include <algorithm>
#include <QTextStream>

const qint64 SECONDS_TO_MILLIS = 1000LL;
const qint64 MINUTES_TO_SECONDS = 60LL;
const qint64 HOURS_TO_SECONDS = MINUTES_TO_SECONDS * 60LL;
const qint64 DAYS_TO_SECONDS = HOURS_TO_SECONDS * 24LL;

QString large_integer_datetime_display_value(const QString &attribute, const QByteArray &bytes, const AdConfig *adconfig);
QString datetime_display_value(const QString &attribute, const QByteArray &bytes, const AdConfig *adconfig);
QString timespan_display_value(const QByteArray &bytes);
QString octet_display_value(const QByteArray &bytes);
QString guid_to_display_value(const QByteArray &bytes);
QString uac_to_display_value(const QByteArray &bytes);
QString samaccounttype_to_display_value(const QByteArray &bytes);
QString primarygrouptype_to_display_value(const QByteArray &bytes);
QString msds_supported_etypes_to_display_value(const QByteArray &bytes);
QString attribute_hex_displayed_value(const QString &attribute, const QByteArray &bytes);

QString attribute_display_value(const QString &attribute, const QByteArray &value, const AdConfig *adconfig) {
    if (adconfig == nullptr) {
        return value;
    }

    const AttributeType type = adconfig->get_attribute_type(attribute);

    switch (type) {
        case AttributeType_Integer: {
            if (attribute == ATTRIBUTE_USER_ACCOUNT_CONTROL) {
                return uac_to_display_value(value);
            } else if (attribute == ATTRIBUTE_SAM_ACCOUNT_TYPE) {
                return samaccounttype_to_display_value(value);
            } else if (attribute == ATTRIBUTE_PRIMARY_GROUP_ID) {
                return primarygrouptype_to_display_value(value);
            } else if (attribute == ATTRIBUTE_GROUP_TYPE || attribute == ATTRIBUTE_SYSTEM_FLAGS || attribute == ATTRIBUTE_MSDS_USER_ACCOUNT_CONTROL_COMPUTED) {
                return attribute_hex_displayed_value(attribute, value);
            } else if (attribute == ATTRIBUTE_MS_DS_SUPPORTED_ETYPES) {
                return msds_supported_etypes_to_display_value(value);
            } else {
                return QString(value);
            }
        }
        case AttributeType_LargeInteger: {
            const LargeIntegerSubtype subtype = adconfig->get_attribute_large_integer_subtype(attribute);
            switch (subtype) {
                case LargeIntegerSubtype_Datetime: return large_integer_datetime_display_value(attribute, value, adconfig);
                case LargeIntegerSubtype_Timespan: return timespan_display_value(value);
                case LargeIntegerSubtype_Integer: return QString(value);
            }

            return QString();
        }
        case AttributeType_UTCTime: return datetime_display_value(attribute, value, adconfig);
        case AttributeType_GeneralizedTime: return datetime_display_value(attribute, value, adconfig);
        case AttributeType_Sid: return object_sid_display_value(value);
        case AttributeType_Octet: {
            if (attribute == ATTRIBUTE_OBJECT_GUID) {
                return guid_to_display_value(value);
            } else {
                return octet_display_value(value);
            }
        }
        case AttributeType_NTSecDesc: {
            return QCoreApplication::translate("attribute_display", "<BINARY VALUE>");
        }
        default: {
            return QString(value);
        }
    }
}

QString attribute_display_values(const QString &attribute, const QList<QByteArray> &values, const AdConfig *adconfig) {
    if (values.isEmpty()) {
        return QCoreApplication::translate("attribute_display", "<unset>");
    } else {
        QString out;

        // Convert values list to
        // "display_value1;display_value2;display_value3..."
        for (int i = 0; i < values.size(); i++) {
            if (i > 0) {
                out += ";";
            }

            const QByteArray value = values[i];
            const QString display_value = attribute_display_value(attribute, value, adconfig);

            out += display_value;
        }

        return out;
    }
}

QString object_sid_display_value(const QByteArray &sid_bytes) {
    dom_sid *sid = (dom_sid *) sid_bytes.data();

    TALLOC_CTX *tmp_ctx = talloc_new(NULL);

    const char *sid_cstr = dom_sid_string(tmp_ctx, sid);
    const QString out = QString(sid_cstr);

    talloc_free(tmp_ctx);

    return out;
}

QString large_integer_datetime_display_value(const QString &attribute, const QByteArray &value, const AdConfig *adconfig) {
    const QString value_string = QString(value);

    if (large_integer_datetime_is_never(value_string)) {
        return QCoreApplication::translate("attribute_display", "(never)");
    } else {
        QDateTime datetime = datetime_string_to_qdatetime(attribute, value_string, adconfig);
        const QString display = datetime.toLocalTime().toString(DATETIME_DISPLAY_FORMAT);

        return display;
    }
}

QString datetime_display_value(const QString &attribute, const QByteArray &bytes, const AdConfig *adconfig) {
    const QString value_string = QString(bytes);
    const QDateTime datetime = datetime_string_to_qdatetime(attribute, value_string, adconfig);
    const QDateTime datetime_local = datetime.toLocalTime();
    const QString display = datetime_local.toString(DATETIME_DISPLAY_FORMAT) + datetime.toLocalTime().timeZoneAbbreviation();

    return display;
}

QString timespan_display_value(const QByteArray &bytes) {
    // Timespan = integer value of hundred nanosecond quantities
    // (also negated)
    // Convert to dd:hh:mm:ss
    const QString value_string = QString(bytes);
    const qint64 hundred_nanos_negative = value_string.toLongLong();

    if (hundred_nanos_negative == LLONG_MIN) {
        return "(never)";
    }

    if (hundred_nanos_negative == 0) {
        return "(none)";
    }

    qint64 seconds_total = [hundred_nanos_negative]() {
        const qint64 hundred_nanos = -hundred_nanos_negative;
        const qint64 millis = hundred_nanos / MILLIS_TO_100_NANOS;
        const qint64 seconds = millis / SECONDS_TO_MILLIS;

        return seconds;
    }();

    const qint64 days = seconds_total / DAYS_TO_SECONDS;
    seconds_total -= days * DAYS_TO_SECONDS;

    const qint64 hours = seconds_total / HOURS_TO_SECONDS;
    seconds_total -= hours * HOURS_TO_SECONDS;

    const qint64 minutes = seconds_total / MINUTES_TO_SECONDS;
    seconds_total -= minutes * MINUTES_TO_SECONDS;

    const qint64 seconds = seconds_total;

    // Convert arbitrary time unit value to string with format
    // "00-99" with leading zero if needed
    const auto time_unit_to_string = [](qint64 time) -> QString {
        if (time > 99) {
            time = 99;
        }

        const QString time_string = QString::number(time);

        if (time == 0) {
            return "00";
        } else if (time < 10) {
            return "0" + time_string;
        } else {
            return time_string;
        }
    };

    const QString days_string = QString::number(days);
    const QString hours_string = time_unit_to_string(hours);
    const QString minutes_string = time_unit_to_string(minutes);
    const QString seconds_string = time_unit_to_string(seconds);

    const QString display = QString("%1:%2:%3:%4").arg(days_string, hours_string, minutes_string, seconds_string);

    return display;
}

QString guid_to_display_value(const QByteArray &bytes) {
    // NOTE: have to do some weird pre-processing to match
    // how Windows displays GUID's. The GUID is broken down
    // into 5 segments, each separated by '-':
    // "0000-11-22-33-444444". Byte order for first 3
    // segments is also reversed (why?), so reverse it again
    // for display.
    const int segments_count = 5;
    QByteArray segments[segments_count];
    segments[0] = bytes.mid(0, 4);
    segments[1] = bytes.mid(4, 2);
    segments[2] = bytes.mid(6, 2);
    segments[3] = bytes.mid(8, 2);
    segments[4] = bytes.mid(10, 6);
    std::reverse(segments[0].begin(), segments[0].end());
    std::reverse(segments[1].begin(), segments[1].end());
    std::reverse(segments[2].begin(), segments[2].end());

    const QString guid_display_string = [&]() {
        QString out;

        for (int i = 0; i < segments_count; i++) {
            const QByteArray segment = segments[i];

            if (i > 0) {
                out += '-';
            }

            out += segment.toHex();
        }

        return out;
    }();

    return guid_display_string;
}

QString octet_display_value(const QByteArray &bytes) {
    const QByteArray bytes_hex = bytes.toHex();

    QByteArray out = bytes_hex;

    // Add " 0x" before each byte (every 2 indexes)
    for (int i = out.size() - 2; i >= 0; i -= 2) {
        out.insert(i, "0x");

        if (i != 0) {
            out.insert(i, " ");
        }
    }

    return QString(out);
}

QString uac_to_display_value(const QByteArray &bytes) {
    bool uac_toInt_ok;
    const int uac = bytes.toInt(&uac_toInt_ok);

    if (!uac_toInt_ok) {
        return QCoreApplication::translate("attribute_display", "<invalid UAC value>");
    }

    // Create string of the form "X | Y | Z", where X,
    // Y, Z are names of masks that are set in given UAC
    const QString masks_string = [&]() {
        // NOTE: using separate list instead of map's
        // keys() because keys() is unordered and we need
        // order so that display string is consistent
        const QList<int> mask_list = {
            UAC_SCRIPT,
            UAC_ACCOUNTDISABLE,
            UAC_HOMEDIR_REQUIRED,
            UAC_LOCKOUT,
            UAC_PASSWD_NOTREQD,
            UAC_PASSWD_CANT_CHANGE,
            UAC_ENCRYPTED_TEXT_PASSWORD_ALLOWED,
            UAC_TEMP_DUPLICATE_ACCOUNT,
            UAC_NORMAL_ACCOUNT,
            UAC_INTERDOMAIN_TRUST_ACCOUNT,
            UAC_WORKSTATION_TRUST_ACCOUNT,
            UAC_SERVER_TRUST_ACCOUNT,
            UAC_DONT_EXPIRE_PASSWORD,
            UAC_MNS_LOGON_ACCOUNT,
            UAC_SMARTCARD_REQUIRED,
            UAC_TRUSTED_FOR_DELEGATION,
            UAC_NOT_DELEGATED,
            UAC_USE_DES_KEY_ONLY,
            UAC_DONT_REQUIRE_PREAUTH,
            UAC_ERROR_PASSWORD_EXPIRED,
            UAC_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION,
            UAC_PARTIAL_SECRETS_ACCOUNT,
            UAC_USER_USE_AES_KEYS,
        };

        const QHash<int, QString> mask_name_map = {
            {UAC_SCRIPT, "SCRIPT"},
            {UAC_ACCOUNTDISABLE, "ACCOUNTDISABLE"},
            {UAC_HOMEDIR_REQUIRED, "HOMEDIR_REQUIRED"},
            {UAC_LOCKOUT, "LOCKOUT"},
            {UAC_PASSWD_NOTREQD, "PASSWD_NOTREQD"},
            {UAC_PASSWD_CANT_CHANGE, "PASSWD_CANT_CHANGE"},
            {UAC_ENCRYPTED_TEXT_PASSWORD_ALLOWED, "ENCRYPTED_TEXT_PASSWORD_ALLOWED"},
            {UAC_TEMP_DUPLICATE_ACCOUNT, "TEMP_DUPLICATE_ACCOUNT"},
            {UAC_NORMAL_ACCOUNT, "NORMAL_ACCOUNT"},
            {UAC_INTERDOMAIN_TRUST_ACCOUNT, "INTERDOMAIN_TRUST_ACCOUNT"},
            {UAC_WORKSTATION_TRUST_ACCOUNT, "WORKSTATION_TRUST_ACCOUNT"},
            {UAC_SERVER_TRUST_ACCOUNT, "SERVER_TRUST_ACCOUNT"},
            {UAC_DONT_EXPIRE_PASSWORD, "DONT_EXPIRE_PASSWORD"},
            {UAC_MNS_LOGON_ACCOUNT, "MNS_LOGON_ACCOUNT"},
            {UAC_SMARTCARD_REQUIRED, "SMARTCARD_REQUIRED"},
            {UAC_TRUSTED_FOR_DELEGATION, "TRUSTED_FOR_DELEGATION"},
            {UAC_NOT_DELEGATED, "NOT_DELEGATED"},
            {UAC_USE_DES_KEY_ONLY, "USE_DES_KEY_ONLY"},
            {UAC_DONT_REQUIRE_PREAUTH, "DONT_REQUIRE_PREAUTH"},
            {UAC_ERROR_PASSWORD_EXPIRED, "ERROR_PASSWORD_EXPIRED"},
            {UAC_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION, "TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION"},
            {UAC_PARTIAL_SECRETS_ACCOUNT, "PARTIAL_SECRETS_ACCOUNT"},
            {UAC_USER_USE_AES_KEYS, "USER_USE_AES_KEYS"},
        };

        const QList<QString> set_mask_name_list = [&]() {
            QList<QString> out_list;

            for (const int mask : mask_list) {
                const bool mask_is_set = bitmask_is_set(uac, mask);

                if (mask_is_set) {
                    const QString mask_name = mask_name_map[mask];
                    out_list.append(mask_name);
                }
            }

            return out_list;
        }();

        const QString out_string = set_mask_name_list.join(" | ");

        return out_string;
    }();

    const QString out = QString("0x%1 = ( %2 )").arg(QString::number((quint32)uac, 16), masks_string);

    return out;
}

QString samaccounttype_to_display_value(const QByteArray &bytes) {
    bool toInt_ok;
    const int value_int = bytes.toInt(&toInt_ok);

    if (!toInt_ok) {
        return QCoreApplication::translate("attribute_display", "<invalid value>");
    }

    const QHash<int, QString> mask_name_map = {
        {SAM_DOMAIN_OBJECT, "DOMAIN_OBJECT"},
        {SAM_GROUP_OBJECT, "GROUP_OBJECT"},
        {SAM_NON_SECURITY_GROUP_OBJECT, "NON_SECURITY_GROUP_OBJECT"},
        {SAM_ALIAS_OBJECT, "ALIAS_OBJECT"},
        {SAM_NON_SECURITY_ALIAS_OBJECT, "NON_SECURITY_ALIAS_OBJECT"},
        {SAM_USER_OBJECT, "USER_OBJECT"},
        {SAM_NORMAL_USER_ACCOUNT, "NORMAL_USER_ACCOUNT"},
        {SAM_MACHINE_ACCOUNT, "MACHINE_ACCOUNT"},
        {SAM_TRUST_ACCOUNT, "TRUST_ACCOUNT"},
        {SAM_APP_BASIC_GROUP, "APP_BASIC_GROUP"},
        {SAM_APP_QUERY_GROUP, "APP_QUERY_GROUP"},
        {SAM_ACCOUNT_TYPE_MAX, "ACCOUNT_TYPE_MAX"},
    };

    const QString mask_name = mask_name_map.value(value_int, "");

    const QString out = QString("%1 = ( %2 )").arg(QString(bytes), mask_name);

    return out;
}

QString primarygrouptype_to_display_value(const QByteArray &bytes) {
    bool toInt_ok;
    const int value_int = bytes.toInt(&toInt_ok);

    if (!toInt_ok) {
        return QCoreApplication::translate("attribute_display", "<invalid value>");
    }

    // NOTE: builtin group rid's are not included
    // because they can't be primary
    const QHash<int, QString> mask_name_map = {
        {DOMAIN_RID_ADMINS, "GROUP_RID_ADMINS"},
        {DOMAIN_RID_USERS, "GROUP_RID_USERS"},
        {DOMAIN_RID_GUESTS, "GROUP_RID_GUESTS"},
        {DOMAIN_RID_DOMAIN_MEMBERS, "GROUP_RID_DOMAIN_MEMBERS"},
        {DOMAIN_RID_DCS, "GROUP_RID_DCS"},
        {DOMAIN_RID_CERT_ADMINS, "GROUP_RID_CERT_ADMINS"},
        {DOMAIN_RID_SCHEMA_ADMINS, "GROUP_RID_SCHEMA_ADMINS"},
        {DOMAIN_RID_ENTERPRISE_ADMINS, "GROUP_RID_ENTERPRISE_ADMINS"},
        {DOMAIN_RID_POLICY_ADMINS, "GROUP_RID_POLICY_ADMINS"},
        {DOMAIN_RID_READONLY_DCS, "GROUP_RID_READONLY_DCS"},
        {DOMAIN_RID_RAS_SERVERS, "GROUP_RID_RAS_SERVERS"},
    };

    if (mask_name_map.contains(value_int)) {
        const QString mask_name = mask_name_map[value_int];
        const QString out = QString("%1 = ( %2 )").arg(QString(bytes), mask_name);

        return out;
    } else {
        return QString::number(value_int);
    }
}

bool attribute_value_is_hex_displayed(const QString &attribute) {
    //TODO: Add here attributes with hex displayed values
    return (attribute == ATTRIBUTE_GROUP_TYPE || attribute == ATTRIBUTE_USER_ACCOUNT_CONTROL ||
            attribute == ATTRIBUTE_MS_DS_SUPPORTED_ETYPES || attribute == ATTRIBUTE_SYSTEM_FLAGS);
}

QString msds_supported_etypes_to_display_value(const QByteArray &bytes) {
    bool toInt_ok;
    const int value_int = bytes.toInt(&toInt_ok);

    if (!toInt_ok) {
        return QCoreApplication::translate("attribute_display", "<invalid value>");
    }

    // NOTE: using separate list instead of map's
    // keys() because keys() is unordered and we need
    // order so that display string is consistent
    const QList<int> mask_list = {
        ETYPES_DES_CBC_CRC,
        ETYPES_DES_CBC_MD5,
        ETYPES_RC4_HMAC_MD5,
        ETYPES_AES128_CTS_HMAC_SHA1_96,
        ETYPES_AES256_CTS_HMAC_SHA1_96
    };

    const QHash<int, QString> mask_name_map = {
        {ETYPES_DES_CBC_CRC, "DES_CBC_CRC"},
        {ETYPES_DES_CBC_MD5, "DES_CBC_MD5"},
        {ETYPES_RC4_HMAC_MD5, "RC4_HMAC_MD5"},
        {ETYPES_AES128_CTS_HMAC_SHA1_96, "AES128_CTS_HMAC_SHA1_96"},
        {ETYPES_AES256_CTS_HMAC_SHA1_96, "AES256_CTS_HMAC_SHA1_96"},
    };

    QStringList masks_strings;
    for (const int mask : mask_list) {
        if (bitmask_is_set(value_int, mask))
            masks_strings.append(mask_name_map[mask]);
    }

    QString display_value = QString("0x%1 = ( %2 )").arg(QString::number((quint32)value_int, 16), masks_strings.join(" | "));
    return display_value;
}

QString attribute_hex_displayed_value(const QString &attribute, const QByteArray &bytes) {
    bool toInt_ok;
    const int value_int = bytes.toInt(&toInt_ok);

    if (!toInt_ok) {
        return QCoreApplication::translate("attribute_display", "<invalid value>");
    }

    const QHash<int, QString> mask_name_map = attribute_value_bit_string_map(attribute);

    QStringList masks_strings;
    for (const int mask : mask_name_map.keys()) {
        if (bitmask_is_set(value_int, mask))
            masks_strings.append(mask_name_map[mask]);
    }

    QString display_value = QString("0x%1 = ( %2 )").arg(QString::number((quint32)value_int, 16), masks_strings.join(" | "));
    return display_value;
}
