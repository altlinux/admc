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

#ifndef MANAGED_BY_TAB_H
#define MANAGED_BY_TAB_H

#include <QWidget>
#include "attribute_edits/attribute_edit.h"

class ManagerEdit;
class ManagedByTabEdit;

namespace Ui {
class ManagedByTab;
}

class ManagedByTab final : public QWidget {
    Q_OBJECT

public:
    Ui::ManagedByTab *ui;

    ManagedByTab(QList<AttributeEdit *> *edit_list, QWidget *parent);
    ~ManagedByTab();
};

class ManagedByTabEdit final : public AttributeEdit {
    Q_OBJECT

public:
    ManagedByTabEdit(QList<AttributeEdit *> *edit_list, Ui::ManagedByTab *ui, QObject *parent);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    Ui::ManagedByTab *ui;
    QList<AttributeEdit *> manager_edits;
    ManagerEdit *manager_edit;

    void on_manager_edited();
    void load_manager_edits(AdInterface &ad);
};

#endif /* MANAGED_BY_TAB_H */
