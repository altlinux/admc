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

#ifndef ATTRIBUTES_TAB_H
#define ATTRIBUTES_TAB_H

/** 
 * Show attributes of target in a list. Allows
 * viewing/editing if possible via attribute editor dialogs.
 */

#include "tabs/properties_tab.h"

#include <QSortFilterProxyModel>
#include <QSet>
#include <QString>

class QStandardItemModel;
class QStandardItem;
class AttributesTabProxy;
class QTreeView;

// NOTE: "readonly" is really "systemonly", it's just that this set of attributes is broken down into "backlink", "constructed" and "systemonly"(aka, not backlink or constructed but still systemonly). Not sure if this is the ideal behavior, maybe change it to be more logical and aligned with what user needs.
enum AttributeFilter {
    AttributeFilter_Unset,
    AttributeFilter_ReadOnly,
    AttributeFilter_Mandatory,
    AttributeFilter_Optional,
    AttributeFilter_SystemOnly,
    AttributeFilter_Constructed,
    AttributeFilter_Backlink,

    AttributeFilter_COUNT,
};


class AttributesTab final : public PropertiesTab {
Q_OBJECT

public:
    AttributesTab();

    void load(AdInterface &ad, const AdObject &object) override;
    void apply(AdInterface &ad, const QString &target) const override;

private slots:
    void edit_attribute();
    void open_filter_dialog();

private:
    QTreeView *view;
    QStandardItemModel *model;
    AttributesTabProxy *proxy;
    QHash<QString, QList<QByteArray>> original;
    QHash<QString, QList<QByteArray>> current;

    void load_row(const QList<QStandardItem *> &row, const QString &attribute, const QList<QByteArray> &values);
};

class AttributesTabProxy final : public QSortFilterProxyModel {

public:
    static QHash<AttributeFilter, bool> default_filters;
    QHash<AttributeFilter, bool> filters;

    AttributesTabProxy(QObject *parent);

    void load(const AdObject &object);

private:
    QSet<QString> set_attributes;
    QSet<QString> mandatory_attributes;
    QSet<QString> optional_attributes;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif /* ATTRIBUTES_TAB_H */
