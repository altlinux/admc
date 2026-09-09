/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include "ad_filter.h"

#include "ad_defines.h"

#include <QCoreApplication>

const QList<QString> filter_classes = {
    CLASS_USER,
    CLASS_GROUP,
    CLASS_CONTACT,
    CLASS_COMPUTER,
    CLASS_PRINTER,
    CLASS_OU,
    CLASS_TRUSTED_DOMAIN,
    CLASS_DOMAIN,
    CLASS_CONTAINER,
    CLASS_INET_ORG_PERSON,
    CLASS_FOREIGN_SECURITY_PRINCIPAL,
    CLASS_SHARED_FOLDER,
    CLASS_RPC_SERVICES,
    CLASS_CERTIFICATE_TEMPLATE,
    CLASS_MSMQ_GROUP,
    CLASS_MSMQ_QUEUE_ALIAS,
    CLASS_REMOTE_STORAGE_SERVICE,
};

QList<QString> process_subfilters(const QList<QString> &in);

QString filter_CONDITION(const Condition condition, const QString &attribute, const QString &value) {
    switch (condition) {
        case Condition_Equals: return QString("(%1=%2)").arg(attribute, value);
        case Condition_NotEquals: return QString("(!(%1=%2))").arg(attribute, value);
        case Condition_StartsWith: return QString("(%1=%2*)").arg(attribute, value);
        case Condition_EndsWith: return QString("(%1=*%2)").arg(attribute, value);
        case Condition_Contains: return QString("(%1=*%2*)").arg(attribute, value);
        case Condition_Set: return QString("(%1=*)").arg(attribute);
        case Condition_Unset: return QString("(!(%1=*))").arg(attribute);
        case Condition_COUNT: return QString();
    }
    return QString();
}

/**
 * @brief Create a filter from the sub-filters list by applying the specified
 * operation.
 * @param operation A filtering operation ("AND"", "OR".)
 * @param subfilters_raw A list of raw sub-filters.
 * @return A filter string.
 */
QString filter_operation(const QString &operation,
                         const QList<QString> &subfilters_raw) {
    const QList<QString> subfilters = process_subfilters(subfilters_raw);

    if (subfilters.size() > 1) {
        QString filter = "(" + operation;
        for (const QString &subfilter : subfilters) {
            filter += subfilter;
        }
        filter += ")";

        return filter;
    } else if (subfilters.size() == 1) {
        return subfilters[0];
    } else {
        return QString();
    }
}

// {x, y, z ...} => (&(x)(y)(z)...)
QString filter_AND(const QList<QString> &subfilters_raw) {
    return filter_operation(FILTER_OPERATION_AND, subfilters_raw);
}

// {x, y, z ...} => (|(x)(y)(z)...)
QString filter_OR(const QList<QString> &subfilters_raw) {
    return filter_operation(FILTER_OPERATION_OR, subfilters_raw);
}

QString condition_to_display_string(const Condition condition) {
    switch (condition) {
        case Condition_Equals: return QCoreApplication::translate("filter", "Is (exactly)");
        case Condition_NotEquals: return QCoreApplication::translate("filter", "Is not");
        case Condition_StartsWith: return QCoreApplication::translate("filter", "Starts with");
        case Condition_EndsWith: return QCoreApplication::translate("filter", "Ends with");
        case Condition_Contains: return QCoreApplication::translate("filter", "Contains");
        case Condition_Set: return QCoreApplication::translate("filter", "Present");
        case Condition_Unset: return QCoreApplication::translate("filter", "Not present");
        case Condition_COUNT: return QString();
    }
    return QString();
}

QList<QString> process_subfilters(const QList<QString> &in) {
    QList<QString> out = in;
    out.removeAll("");

    return out;
}

/**
 * @brief Make a filter with the specified condition and an attribute type to
 * the given attribute list.
 * @param condition A filtering condition.
 * @param attribute A filtering attribute.
 * @param attribute_list A list of attributes to filter.
 * @return A list of filters that can be further used with procedures
 * like "filter_OR".
 */
const QList<QString> make_filter_list(const Condition &condition,
                                      const QString &attribute,
                                      const QList<QString> &attribute_list) {
    QList<QString> subfilter_list;
    for (const QString &attr : attribute_list) {
        const QString subfilter = filter_CONDITION(condition, attribute, attr);
        subfilter_list.append(subfilter);
    }
    return subfilter_list;
}

/**
 * @brief Apply a filter with the specified condition and attribute type to the
 * given attribute list.
 * @param filter A filter procedure to be applied.
 * @param condition A filtering condition.
 * @param attribute A filtering attribute.
 * @param attribute_list A list of attributes to filter.
 * @return The newly created filter.
 */
QString filter_attributes(filter_t filter,
                          const Condition &condition,
                          const QString &attribute,
                          const QList<QString> &attribute_list) {
    const QList<QString> subfilter_list = make_filter_list(condition,
                                                           attribute,
                                                           attribute_list);
    return filter(subfilter_list);
}

QString filter_dn_list(const QList<QString> &dn_list) {
    return filter_attributes(filter_OR, Condition_Equals, ATTRIBUTE_DN,
                             dn_list);
}

QString filter_matching_rule_in_chain(const QString &attribute, const QString &dn_value) {
    const QString filter = attribute + ":" + QString(MATCHING_RULE_IN_CHAIN_OID) + ":=" + dn_value;
    return QString("(" + filter + ")");
}
