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

#ifndef ATTRIBUTES_TAB_H
#define ATTRIBUTES_TAB_H

/**
 * Show attributes of target in a list. Allows
 * viewing/editing if possible via attribute dialogs.
 */

#include "attribute_edits/attribute_edit.h"
#include <QWidget>

enum AttributesColumn {
    AttributesColumn_Name,
    AttributesColumn_Value,
    AttributesColumn_Type,
    AttributesColumn_COUNT,
};

class QStandardItemModel;
class QStandardItem;
class AttributesTabProxy;
class AttributesFilterDialog;
class AttributeDialog;
class AttributesTabEdit;
class QTreeView;
class QPushButton;

namespace Ui {
class AttributesTab;
}

class AttributesTab final : public QWidget {
    Q_OBJECT

public:
    Ui::AttributesTab *ui;

    AttributesTab(QList<AttributeEdit *> *edit_list, QWidget *parent);
    ~AttributesTab();
};

class AttributesTabEdit final : public AttributeEdit {
    Q_OBJECT

public:
    AttributesTabEdit(QTreeView *view, QPushButton *filter_button, QPushButton *edit_button, QPushButton *view_button,
                      QPushButton *load_optional_attrs_button_arg, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;
    bool apply(AdInterface &ad, const QString &dn) const override;

private:
    QTreeView *view;
    QPushButton *filter_button;
    QPushButton *edit_button;
    QPushButton *view_button;
    QPushButton *load_optional_attrs_button;

    AttributesFilterDialog *filter_dialog;
    QStandardItemModel *model;
    AttributesTabProxy *proxy;
    QHash<QString, QList<QByteArray>> original;
    QHash<QString, QList<QByteArray>> current;
    QList<QString> not_specified_optional_attributes;
    QString object_dn;
    bool optional_attrs_values_is_loaded;

    void update_edit_and_view_buttons();
    void on_double_click();
    void edit_attribute();
    void view_attribute();
    void on_load_optional();
    bool eventFilter(QObject *watched, QEvent *event) override;
    void copy_action();
    void load_optional_attribute_values(AdInterface &ad);
    void load_row(const QList<QStandardItem *> &row, const QString &attribute, const QList<QByteArray> &values);
    QList<QStandardItem *> get_selected_row() const;
    AttributeDialog *get_attribute_dialog(const bool read_only);
    void reload_model();
};

#endif /* ATTRIBUTES_TAB_H */
