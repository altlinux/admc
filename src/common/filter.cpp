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

#include "filter.h"
#include "settings.h"
#include "ad_interface.h"

QString filter_EQUALS(const QString &attribute, const QString &value) {
    auto filter = QString("(%1=%2)").arg(attribute, value);
    return filter;
}

// {x, y, z ...} => (&(x)(y)(z)...)
QString filter_AND(const QList<QString> &subfilters) {
    if (!subfilters.isEmpty()) {
        QString filter = "(&";
        for (const QString subfilter : subfilters) {
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
        for (const QString subfilter : subfilters) {
            filter += subfilter;
        }
        filter += ")";

        return filter;
    } else {
        return QString();
    }
}

QString filter_NOT(const QString &filter) {
    return QString("(!%1)").arg(filter);
}

QString current_advanced_view_filter() {
    const bool advanced_view = SETTINGS()->get_bool(BoolSetting_AdvancedView);

    if (advanced_view) {
        return QString();
    } else {
        return filter_NOT(filter_EQUALS(ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY, "true"));
    }
}
