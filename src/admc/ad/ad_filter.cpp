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

#include "ad/ad_filter.h"

#include "ad/ad_defines.h"
#include "ad/ad_config.h"
#include "globals.h"

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

QString filter_CONDITION(const Condition condition, const QString &attribute, const QString &value) {
    switch(condition) {
        case Condition_Equals: return QString("(%1=%2)").arg(attribute, value);
        case Condition_NotEquals: return QString("(!(%1=%2))").arg(attribute, value);
        case Condition_StartsWith: return QString("(%1=*%2)").arg(attribute, value);
        case Condition_EndsWith: return QString("(%1=%2*)").arg(attribute, value);
        case Condition_Contains: return QString("(%1=*%2*)").arg(attribute, value);
        case Condition_Set: return QString("(%1=*)").arg(attribute);
        case Condition_Unset: return QString("(!(%1=*))").arg(attribute);
        case Condition_COUNT: return QString();
    }
    return QString();
}

// {x, y, z ...} => (&(x)(y)(z)...)
QString filter_AND(const QList<QString> &subfilters) {
    if (!subfilters.isEmpty()) {
        QString filter = "(&";
        for (const QString &subfilter : subfilters) {
            filter += subfilter;
        }
        filter += ")";

        return filter;
    } else {
        return QString();
    }
}

// {x, y, z ...} => (|(x)(y)(z)...)
QString filter_OR(const QList<QString> &subfilters) {
    if (!subfilters.isEmpty()) {
        QString filter = "(|";
        for (const QString &subfilter : subfilters) {
            filter += subfilter;
        }
        filter += ")";

        return filter;
    } else {
        return QString();
    }
}

QString is_container_filter() {
    const QList<QString> accepted_classes = adconfig->get_filter_containers();

    QList<QString> class_filters;
    for (const QString &object_class : accepted_classes) {
        const QString class_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, object_class);
        class_filters.append(class_filter);
    }

    return filter_OR(class_filters);
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
