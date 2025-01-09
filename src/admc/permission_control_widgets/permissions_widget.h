/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#ifndef PERMISSIONS_WIDGET_H
#define PERMISSIONS_WIDGET_H

#include <QWidget>
#include <QLocale>
#include <QSortFilterProxyModel>

class QStandardItemModel;
class QStandardItem;
struct security_descriptor;
class QTreeView;
class AdInterface;
class QSortFilterProxyModel;
class QModelIndex;
class QVBoxLayout;
struct SecurityRight;
class QSortFilterProxyModel;


enum RightsItemRole {
    RightsItemRole_SecurityRight = Qt::UserRole + 1,
    RightsItemRole_SecurityRightList,
    // ObjectTypeName serves as item data type for sorting purpose
    RightsItemRole_ObjectTypeName,
    RightsItemRole_ItemType,
    RightsItemRole_HiddenItem
};

enum AppliedObjects {
    AppliedObjects_ThisObject,
    AppliedObjects_ThisAndChildObjects,
    AppliedObjects_AllChildObjects,
    AppliedObjects_ChildObjectClass
};

class RightsSortModel : public QSortFilterProxyModel {
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    void hide_ignored_items();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

class PermissionsWidget : public QWidget {
    Q_OBJECT

public:
    explicit PermissionsWidget(QWidget *parent = nullptr);
    virtual ~PermissionsWidget() = default;

    virtual void init(const QStringList &target_classes,
                      security_descriptor *sd_arg);
    virtual void set_read_only();
    void set_current_trustee(const QByteArray &current_trustee);
    // Updates permissions for given applied objects case
    virtual void update_permissions(AppliedObjects applied_objs, const QString &appliable_child_class = QString());
    // Updates permissions with current applied objects value
    virtual void update_permissions();
    bool there_are_selected_permissions() const;

signals:
    void edited();

protected:
    enum PermissionColumn {
        PermissionColumn_Name,
        PermissionColumn_Allowed,
        PermissionColumn_Denied,

        PermissionColumn_COUNT,
    };

    bool ignore_item_changed_signal;
    security_descriptor *sd;
    bool read_only;
    QStandardItemModel *rights_model;
    QTreeView *rights_view;
    QByteArray trustee;
    QStringList target_class_list;
    QLocale::Language language;
    RightsSortModel *rights_sort_model;
    QVBoxLayout *v_layout;
    AppliedObjects applied_objects = AppliedObjects_ThisObject;
    QPersistentModelIndex message_index;
    QString appliable_class;

    virtual void on_item_changed(QStandardItem *item);
    virtual void make_model_rights_read_only();

    void show_no_rights_message(bool show);
    bool item_is_message(const QModelIndex &index) const;
    void append_message_item();
    virtual QList<QStandardItem*> create_item_row(const SecurityRight &right);

private:
    void update_row_check_state(int row, const SecurityRight &right);
    virtual bool right_applies_to_class(const SecurityRight &right, const QString &obj_class) = 0;
    virtual bool there_are_rights_for_class(const QString &obj_class) = 0;
};

#endif // PERMISSIONS_WIDGET_H
