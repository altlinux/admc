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

#include "attribute_display.h"
#include "ad_defines.h"
#include "ad_utils.h"
#include "ad_config.h"

QString large_integer_datetime_to_display_value(const QString &attribute, const QByteArray &bytes);
QString datetime_to_display_value(const QString &attribute, const QByteArray &bytes);
QString timespan_to_display_value(const QByteArray &bytes);
QString octet_to_display_value(const QString &attribute, const QByteArray &bytes);
QString guid_to_display_value(const QByteArray &bytes);

QString attribute_display_value(const QString &attribute, const QByteArray &value) {
    const AttributeType type = ADCONFIG()->get_attribute_type(attribute);

    switch (type) {
        case AttributeType_LargeInteger: {
            const LargeIntegerSubtype subtype = ADCONFIG()->get_attribute_large_integer_subtype(attribute);

            switch (subtype) {
                case LargeIntegerSubtype_Datetime: return large_integer_datetime_to_display_value(attribute, value);
                case LargeIntegerSubtype_Timespan: return timespan_to_display_value(value);
                case LargeIntegerSubtype_Integer: return QString(value);
            }
        }
        case AttributeType_UTCTime: return datetime_to_display_value(attribute, value);
        case AttributeType_GeneralizedTime: return datetime_to_display_value(attribute, value);
        case AttributeType_Sid: return object_sid_to_display_value(value);
        case AttributeType_Octet: {
            if (attribute == ATTRIBUTE_OBJECT_GUID) {
                return guid_to_display_value(value);
            } else {
                return octet_to_display_value(value);
            }
        }
        default: {
            return QString(value);
        }
    }
}

QString attribute_display_values(const QString &attribute, const QList<QByteArray> &values) {
    if (values.isEmpty()) {
        return QObject::tr("<unset>");
    } else {
        QString out;

        // Convert values list to
        // "display_value1;display_value2;display_value3..."
        for (int i = 0; i < values.size(); i++) {
            if (i > 0) {
                out += ";";
            }

            const QByteArray value = values[i];
            const QString display_value = attribute_display_value(attribute, value);

            out += display_value;
        }

        return out;
    }
}

// TODO: replace with some library if possible. Maybe one of samba's libs has this.
// NOTE: https://ldapwiki.com/wiki/ObjectSID
QString object_sid_to_display_value(const QByteArray &sid) {
    QString string = "S-";
    
    // byte[0] - revision level
    const int revision = sid[0];
    string += QString::number(revision);
    
    // byte[1] - count of sub-authorities
    const int sub_authority_count = sid[1] & 0xFF;
    
    // byte(2-7) - authority 48 bit Big-Endian
    long authority = 0;
    for (int i = 2; i <= 7; i++) {
        authority |= ((long)sid[i]) << (8 * (5 - (i - 2)));
    }
    string += "-" + QString::number(authority);
    // TODO: not sure if authority is supposed to be formatted as hex or not. Currently it matches how ADUC displays it.
    
    // byte(8-...)
    // sub-authorities 32 bit Little-Endian
    int offset = 8;
    const int bytes = 4;
    for (int i = 0; i < sub_authority_count; i++) {
        long sub_authority = 0;
        for (int j = 0; j < bytes; j++) {
            sub_authority |= (long)(sid[offset + j] & 0xFF) << (8 * j);
        }

        string += "-" + QString::number(sub_authority);

        offset += bytes;
    }

    return string;
}

QString large_integer_datetime_to_display_value(const QString &attribute, const QByteArray &value) {
    const QString value_string = QString(value);
    
    if (large_integer_datetime_is_never(value_string)) {
        return QObject::tr("(never)");
    } else {
        QDateTime datetime = datetime_string_to_qdatetime(attribute, value_string);
        const QString display = datetime.toLocalTime().toString(DATETIME_DISPLAY_FORMAT);

        return display;
    }
}

QString datetime_to_display_value(const QString &attribute, const QByteArray &bytes) {
    const QString value_string = QString(bytes);
    const QDateTime datetime = datetime_string_to_qdatetime(attribute, value_string);
    const QDateTime datetime_local = datetime.toLocalTime();
    const QString display = datetime_local.toString(DATETIME_DISPLAY_FORMAT) + datetime.toLocalTime().timeZoneAbbreviation();

    return display;
}

QString timespan_to_display_value(const QByteArray &bytes) {
    // Timespan = integer value of hundred nanosecond quantities
    // (also negated)
    // Convert to dd:hh:mm:ss
    const QString value_string = QString(bytes);
    const qint64 hundred_nanos_negative = value_string.toLongLong();

    if (hundred_nanos_negative == LLONG_MIN) {
        return "(never)";
    }

    qint64 seconds_total =
    [hundred_nanos_negative]() {
        const qint64 hundred_nanos = -hundred_nanos_negative;
        const qint64 millis = hundred_nanos / MILLIS_TO_100_NANOS;
        const qint64 seconds = millis / SECONDS_TO_MILLIS;

        return seconds;
    }();

    const qint64 days = seconds_total / DAYS_TO_SECONDS;
    seconds_total = days % DAYS_TO_SECONDS;

    const qint64 hours = seconds_total / HOURS_TO_SECONDS; 
    seconds_total = hours % HOURS_TO_SECONDS;

    const qint64 minutes = seconds_total / MINUTES_TO_SECONDS; 
    seconds_total = minutes % MINUTES_TO_SECONDS;

    const qint64 seconds = seconds_total;

    // Convert arbitrary time unit value to string with format
    // "00-99" with leading zero if needed
    const auto time_unit_to_string =
    [](qint64 time) -> QString {
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

    const QString days_string = time_unit_to_string(days);
    const QString hours_string = time_unit_to_string(hours);
    const QString minutes_string = time_unit_to_string(minutes);
    const QString seconds_string = time_unit_to_string(seconds);

    const QString display = QString("%1:%2:%3:%4").arg(days_string, hours_string, minutes_string, seconds_string);

    return display;
}

// To match how ADUC displays GUID's, need to swap
// some bytes and insert hyphens
// TODO: no idea what the logic behind these byte swaps is, figure it out
QString guid_to_display_value(const QByteArray &bytes) {
    QByteArray out = bytes;

    const auto swap_bytes =
    [&out, bytes](const int a, const int b) {
        out[a] = bytes[b];
        out[b] = bytes[a];
    };

    swap_bytes(0, 3);
    swap_bytes(1, 2);
    swap_bytes(4, 5);
    swap_bytes(6, 7);

    out = out.toHex();

    out.insert(20, '-');
    out.insert(16, '-');
    out.insert(12, '-');
    out.insert(8, '-');

    return QString(out);
}

QString octet_to_display_value(const QByteArray &bytes) {
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
