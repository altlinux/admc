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

#ifndef MEMBERS_TAB_H
#define MEMBERS_TAB_H

#include "tabs/properties_tab.h"

#include <QSet>

class QStandardItemModel;

// Displays and edits membership info which can go both ways
// 1. users that are members of group
// 2. groups of user is member of
// MembersTab and MemberOfTab implement both of those

namespace Ui {
    class MembershipTab;
}

class MembershipTab : public PropertiesTab {
    Q_OBJECT

public:
    void load(AdInterface &ad, const AdObject &object) override;
    bool apply(AdInterface &ad, const QString &target) override;

protected:
    enum MembershipTabType {
        MembershipTabType_Members,
        MembershipTabType_MemberOf
    };

    MembershipTab(const MembershipTabType type_arg);
    ~MembershipTab();

private:
    Ui::MembershipTab *ui;
    MembershipTabType type;
    QStandardItemModel *model;

    QSet<QString> original_values;
    QSet<QString> original_primary_values;
    QSet<QString> current_values;
    QSet<QString> current_primary_values;

    void on_add_button();
    void on_remove_button();
    void on_primary_button();
    void on_properties_button();
    void enable_primary_button_on_valid_selection();
    void reload_model();
    void add_values(QList<QString> values);
    void remove_values(QList<QString> values);
    QString get_membership_attribute();
};

class MembersTab final : public MembershipTab {
public:
    MembersTab();
};

class MemberOfTab final : public MembershipTab {
public:
    MemberOfTab();
};

#endif /* MEMBERS_TAB_H */
