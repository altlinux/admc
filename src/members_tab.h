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

#ifndef MEMBERS_TAB_H
#define MEMBERS_TAB_H

#include "details_tab.h"
#include "object_model.h"

class QTreeView;
class QString;
class ObjectContextMenu;
class MembersModel;

// Shows member objects of targeted group
class MembersTab final : public DetailsTab {
Q_OBJECT

public:
    MembersTab(ObjectContextMenu *object_context_menu, DetailsWidget *details_arg);

    void apply();
    bool accepts_target() const;

private:
    MembersModel *model = nullptr;
    QTreeView *view = nullptr;

    void reload_internal();
};

class MembersModel final : public ObjectModel {
Q_OBJECT

public:
    MembersModel(QObject *parent);

    void change_target(const QString &dn);
};

#endif /* MEMBERS_TAB_H */
