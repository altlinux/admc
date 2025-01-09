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

#ifndef MEMBERS_TAB_H
#define MEMBERS_TAB_H

#include "attribute_edits/attribute_edit.h"
#include <QWidget>

#include <QSet>

class QStandardItemModel;
class QTreeView;
class QPushButton;
class QLabel;

// Displays and edits membership info which can go both ways
// 1. users that are members of group
// 2. groups of user is member of
// MembersTab and MemberOfTab implement both of those

enum MembershipTabType {
    MembershipTabType_Members,
    MembershipTabType_MemberOf
};

namespace Ui {
class MembershipTab;
}

class MembershipTab final : public QWidget {
    Q_OBJECT

public:
    Ui::MembershipTab *ui;

    MembershipTab(QList<AttributeEdit *> *edit_list, const MembershipTabType &type, QWidget *parent);
    ~MembershipTab();
};

class MembershipTabEdit final : public AttributeEdit {
    Q_OBJECT

public:
    MembershipTabEdit(QTreeView *view, QPushButton *primary_button, QPushButton *add_button, QPushButton *remove_button, QPushButton *properties_button, QLabel *primary_group_label, const MembershipTabType &type, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;
    bool apply(AdInterface &ad, const QString &dn) const override;

private:
    QTreeView *view;
    QPushButton *primary_button;
    QPushButton *add_button;
    QPushButton *remove_button;
    QPushButton *properties_button;
    QLabel *primary_group_label;
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

#endif /* MEMBERS_TAB_H */
