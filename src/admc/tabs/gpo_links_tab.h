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

#ifndef GPO_LINKS_TAB_H
#define GPO_LINKS_TAB_H

#include "tabs/properties_tab.h"

class QTreeView;
class QStandardItemModel;

/**
 * List objects that this GPO links to. Does not provide a
 * way to edit, editing should be done in group policy tabs
 * of objects.
 */

class GpoLinksTab final : public PropertiesTab {
    Q_OBJECT

public:
    GpoLinksTab();
    ~GpoLinksTab();

    void load(AdInterface &ad, const AdObject &object) override;

private:
    QTreeView *view;
    QStandardItemModel *model;
};

#endif /* GPO_LINKS_TAB_H */
