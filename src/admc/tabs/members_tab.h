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

#include "tabs/details_tab.h"
#include "object_model.h"

#include <QPoint>
#include <QSet>
#include <QString>

class QTreeView;
class QString;
class ObjectContextMenu;
class MembersModel;

class QStandardItemModel;

// Displays and edits membership info which can go both ways
// 1. users that are members of group
// 2. groups of user is member of
// MembersTab and MemberOfTab implement both of those

class MembershipTab : public DetailsTab {
Q_OBJECT

public:
    DECL_DETAILS_TAB_VIRTUALS();

private slots:
    void on_context_menu(const QPoint pos);
    void on_add_button();
    void on_remove_button();

protected:
    enum MembershipTabType {
        MembershipTabType_Members,
        MembershipTabType_MemberOf
    };

    MembershipTab(const AdObject &object, const MembershipTabType type_arg);

private:
    MembershipTabType type;
    QStandardItemModel *model = nullptr;
    QTreeView *view = nullptr;
    QSet<QString> original_values;
    QSet<QString> current_values;

    void reload_current_values_into_model();
    void add_values(QList<QString> values);
    void remove_values(QList<QString> values);
    QString get_membership_attribute();
};

class MembersTab final : public MembershipTab {
public:
    MembersTab(const AdObject &object);
};

class MemberOfTab final : public MembershipTab {
public:
    MemberOfTab(const AdObject &object);
};

#endif /* MEMBERS_TAB_H */
