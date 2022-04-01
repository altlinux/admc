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

#ifndef GROUP_POLICY_TAB_H
#define GROUP_POLICY_TAB_H

#include <QWidget>
#include "attribute_edits/attribute_edit.h"

#include "gplink.h"

class QString;
class QStandardItemModel;
class QStandardItem;
class QPoint;
class GroupPolicyTabEdit;

/**
 * Tab for displaying, modifying group policy related
 * attributes of an object(not a gpo!), such as gplink and
 * gpoptions.
 */

namespace Ui {
class GroupPolicyTab;
}

class GroupPolicyTab final : public QWidget {
    Q_OBJECT

public:
    Ui::GroupPolicyTab *ui;

    GroupPolicyTab(QList<AttributeEdit *> *edit_list, QWidget *parent);
    ~GroupPolicyTab();
};

class GroupPolicyTabEdit final : public AttributeEdit {
    Q_OBJECT

public:
    GroupPolicyTabEdit(Ui::GroupPolicyTab *ui, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;
    bool apply(AdInterface &ad, const QString &dn) const override;

private:
    Ui::GroupPolicyTab *ui;
    QStandardItemModel *model;
    Gplink gplink;
    QString original_gplink_string;

    void on_context_menu(const QPoint pos);
    void on_remove_button();
    void on_item_changed(QStandardItem *item);
    void on_add_button();

    void add_link(QList<QString> gpos);
    void remove_link(QList<QString> gpos);
    void move_link_up(const QString &gpo);
    void move_link_down(const QString &gpo);

    void reload_gplink();
};

#endif /* GROUP_POLICY_TAB_H */
