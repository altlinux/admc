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

#ifndef ATTRIBUTES_TAB_PROXY_H
#define ATTRIBUTES_TAB_PROXY_H

#include <QSet>
#include <QSortFilterProxyModel>

class AttributesTabFilterMenu;
class AdObject;

class AttributesTabProxy final : public QSortFilterProxyModel {

public:
    AttributesTabProxy(AttributesTabFilterMenu *filter_menu, QObject *parent);

    void load(const AdObject &object);
    void update_set_attributes(QSet<QString> attributes);

private:
    AttributesTabFilterMenu *filter_menu;
    QSet<QString> set_attributes;
    QSet<QString> mandatory_attributes;
    QSet<QString> optional_attributes;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif /* ATTRIBUTES_TAB_PROXY_H */
