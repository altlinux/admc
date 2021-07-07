/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#ifndef ATTRIBUTES_TAB_P_H
#define ATTRIBUTES_TAB_P_H

#include <QDialog>
#include <QSortFilterProxyModel>
#include <QSet>

class QCheckBox;

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

class AttributesFilterDialog final : public QDialog {
    Q_OBJECT

public:
    AttributesFilterDialog(QWidget *parent);

    void accept() override;

private:
    QHash<AttributeFilter, QCheckBox *> checks;

    void on_read_only_check();
};

class AttributesTabProxy final : public QSortFilterProxyModel {

public:
    AttributesTabProxy(QObject *parent);

    void update_filter_state();
    void load(const AdObject &object);

private:
    QSet<QString> set_attributes;
    QSet<QString> mandatory_attributes;
    QSet<QString> optional_attributes;
    QHash<AttributeFilter, bool> filter_state;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif /* ATTRIBUTES_TAB_P_H */
