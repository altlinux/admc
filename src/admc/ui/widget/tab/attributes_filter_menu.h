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

#ifndef ATTRIBUTES_TAB_FILTER_MENU_H
#define ATTRIBUTES_TAB_FILTER_MENU_H

#include <QHash>
#include <QMenu>

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

class AttributesTabFilterMenu final : public QMenu {
    Q_OBJECT

public:
    AttributesTabFilterMenu(QWidget *parent);
    ~AttributesTabFilterMenu();

    bool filter_is_enabled(const AttributeFilter filter) const;

signals:
    void filter_changed();

private:
    QHash<AttributeFilter, QAction *> action_map;

    void on_read_only_changed();
};

#endif /* ATTRIBUTES_TAB_FILTER_MENU_H */
