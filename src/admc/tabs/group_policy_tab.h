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

#include "tabs/details_tab.h"
#include "gplink.h"

#include <QPoint>

class QTreeView;
class QString;
class QStandardItemModel;
class QStandardItem;

// Tab for displaying, modifying group policy related attributes of an object(not a gpo!), such as gplink and gpoptions

class GroupPolicyTab final : public DetailsTab {
Q_OBJECT

public:
    GroupPolicyTab();

    void load(const AdObject &object) override;
    void apply(const QString &target) const override;

private slots:
    void on_context_menu(const QPoint pos);
    void on_add_button();
    void on_remove_button();
    void on_item_changed(QStandardItem *item);

private:
    QStandardItemModel *model = nullptr;
    QTreeView *view = nullptr;
    Gplink gplink;

    void add_link(QList<QString> gpos);
    void remove_link(QList<QString> gpos);
    void move_link_up(const QString &gpo);
    void move_link_down(const QString &gpo);

    void reload_gplink();
};

#endif /* GROUP_POLICY_TAB_H */
