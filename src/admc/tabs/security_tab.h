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

#ifndef SECURITY_TAB_H
#define SECURITY_TAB_H

#include <QWidget>
#include "attribute_edits/attribute_edit.h"

#include "ad_defines.h"

#include <QDialog>

class RightsSortModel;
class QStandardItemModel;
class QStandardItem;
class SecurityDescriptor;
struct security_descriptor;

enum AceColumn {
    AceColumn_Name,
    AceColumn_Allowed,
    AceColumn_Denied,

    AceColumn_COUNT,
};

class SecurityTabEdit;

namespace Ui {
class SecurityTab;
}

class SecurityTab final : public QWidget {
    Q_OBJECT

public:
    Ui::SecurityTab *ui;

    SecurityTab(QList<AttributeEdit *> *edit_list, QWidget *parent);
    ~SecurityTab();

    void fix_acl_order();
    void set_read_only();
    bool verify_acl_order() const;

private:
    SecurityTabEdit *tab_edit;
};

class SecurityTabEdit final : public AttributeEdit {
    Q_OBJECT

public:
    SecurityTabEdit(Ui::SecurityTab *ui, QObject *parent);
    ~SecurityTabEdit();

    void load(AdInterface &ad, const AdObject &object) override;
    bool verify(AdInterface &ad, const QString &dn) const override;
    bool apply(AdInterface &ad, const QString &dn) const override;

    void fix_acl_order();
    void set_read_only();
    bool verify_acl_order() const;

private:
    Ui::SecurityTab *ui;
    QStandardItemModel *trustee_model;
    QStandardItemModel *rights_model;
    RightsSortModel *rights_sort_model;
    bool is_policy;
    bool ignore_item_changed_signal;
    security_descriptor *sd;
    QList<QString> target_class_list;
    bool read_only;

    void on_item_changed(QStandardItem *item);
    void on_add_trustee_button();
    void on_remove_trustee_button();
    void add_trustees(const QList<QByteArray> &sid_list, AdInterface &ad);
    void on_add_well_known_trustee();
    void load_current_sd(AdInterface &ad);
    void load_rights_model();
    void make_rights_model_read_only();
    QByteArray get_current_trustee() const;
};

#endif /* SECURITY_TAB_H */
