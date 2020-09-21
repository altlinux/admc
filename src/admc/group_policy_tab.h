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

#ifndef GROUP_POLICY_TAB_H
#define GROUP_POLICY_TAB_H

#include "details_tab.h"
#include "gplink.h"

#include <QPoint>
#include <QList>

class QTreeView;
class QString;
class ObjectContextMenu;
class MembersModel;
class QStandardItemModel;
class AttributeEdit;
class Gplink;

// Tab for displaying, modifying group policy related attributes of an object(not a gpo!), such as gplink and gpoptions

class GroupPolicyTab final : public DetailsTab {
Q_OBJECT

public:
    GroupPolicyTab(DetailsWidget *details_arg);
    DECL_DETAILS_TAB_VIRTUALS();

private slots:
    void on_context_menu(const QPoint pos);
    void on_add_button();
    void on_remove_button();

private:
    QStandardItemModel *model = nullptr;
    QTreeView *view = nullptr;
    Gplink original_gplink;
    Gplink current_gplink;

    QList<AttributeEdit *> edits;

    void add(QList<QString> gpos);
    void remove(QList<QString> gpos);
    void edited();
};

#endif /* GROUP_POLICY_TAB_H */
