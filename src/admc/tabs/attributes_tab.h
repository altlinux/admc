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

#include "tabs/details_tab.h"

#include <QStandardItemModel>
#include <QString>

class AttributesModel;
class QTreeView;

// Show attributes of target as a list of attribute names and values
// Values are editable
class AttributesTab final : public DetailsTab {
Q_OBJECT

public:
    AttributesTab();
    DECL_DETAILS_TAB_VIRTUALS();

private:
    AttributesModel *model = nullptr;
    QTreeView *view = nullptr;
};

class AttributesModel final : public QStandardItemModel {
Q_OBJECT

public:
    explicit AttributesModel(AttributesTab *attributes_tab_arg);

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    void reload(const AdObject &object);

private:
    AttributesTab *attributes_tab;
};

#endif /* ATTRIBUTES_TAB_H */
