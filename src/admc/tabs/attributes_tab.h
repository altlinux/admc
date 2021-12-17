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

#ifndef ATTRIBUTES_TAB_H
#define ATTRIBUTES_TAB_H

/** 
 * Show attributes of target in a list. Allows
 * viewing/editing if possible via attribute dialogs.
 */

#include <QWidget>
#include "attribute_edits/attribute_edit.h"

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
    AttributesTabEdit(QList<AttributeEdit *> *edit_list, Ui::AttributesTab *ui, QObject *parent);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    Ui::AttributesTab *ui;
    AttributesFilterDialog *filter_dialog;
    QStandardItemModel *model;
    AttributesTabProxy *proxy;
    QHash<QString, QList<QByteArray>> original;
    QHash<QString, QList<QByteArray>> current;

    void update_edit_and_view_buttons();
    void on_double_click();
    void edit_attribute();
    void view_attribute();
    void load_row(const QList<QStandardItem *> &row, const QString &attribute, const QList<QByteArray> &values);
    QList<QStandardItem *> get_selected_row() const;
    AttributeDialog *get_attribute_dialog(const bool read_only);
};

#endif /* ATTRIBUTES_TAB_H */
