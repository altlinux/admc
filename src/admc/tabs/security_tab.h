/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "attribute_edits/attribute_edit.h"
#include <QWidget>

class SecurityTabEdit;
class QStandardItemModel;
struct security_descriptor;
class PermissionsWidget;
class SDDLViewDialog;

namespace Ui {
class SecurityTab;
}

class SecurityTab final : public QWidget {
    Q_OBJECT

    friend class SecurityTabEdit;

    enum TrusteeItemRole {
        TrusteeItemRole_Sid = Qt::UserRole,
    };

    enum AppliedObjectRole {
        AppliedObjectRole_ObjectClass = Qt::UserRole + 1
    };

public:
    SecurityTab(QList<AttributeEdit *> *edit_list, QWidget *parent);
    ~SecurityTab();

    void fix_acl_order();
    void set_read_only();
    bool verify_acl_order() const;

private:
    Ui::SecurityTab *ui;
    SecurityTabEdit *tab_edit;
    QStandardItemModel *trustee_model;
    security_descriptor *sd;
    security_descriptor *previous_sd;
    bool is_policy;
    QList<PermissionsWidget*> permissions_widgets;
    SDDLViewDialog *sddl_view;
    QAction *restore_sd_action;
    QStringList target_class_list;

    void load(AdInterface &ad, const AdObject &object);
    void on_remove_trustee_button();
    void on_add_trustee_button();
    void on_add_well_known_trustee();
    void add_trustees(const QList<QByteArray> &sid_list, AdInterface &ad);
    void load_sd(AdInterface &ad, security_descriptor *sd_arg);
    QByteArray get_current_trustee() const;
    void load_applied_objects_cmbbox(const QStringList &target_class_list);
    void on_applied_objs_cmbbox();
    void on_clear_all();
    void on_show_sddl_sd();
    void on_restore_previous_sd();
    void on_more_menu();

signals:
    void current_trustee_changed(const QByteArray &current_trustee);
    void trustees_removed(const QByteArrayList &trustees);
};

class SecurityTabEdit final : public AttributeEdit {
    Q_OBJECT

public:
    SecurityTabEdit(SecurityTab *security_tab_arg);
    ~SecurityTabEdit();

    void load(AdInterface &ad, const AdObject &object) override;
    bool verify(AdInterface &ad, const QString &dn) const override;
    bool apply(AdInterface &ad, const QString &dn) const override;

private:
    SecurityTab *security_tab;
};

#endif /* SECURITY_TAB_H */
