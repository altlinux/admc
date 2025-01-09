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

#ifndef ORGANIZATION_TAB_H
#define ORGANIZATION_TAB_H

#include "attribute_edits/attribute_edit.h"
#include <QWidget>

class QStandardItemModel;

namespace Ui {
class OrganizationTab;
}

class OrganizationTab final : public QWidget {
    Q_OBJECT

public:
    Ui::OrganizationTab *ui;

    OrganizationTab(QList<AttributeEdit *> *edit_list, QWidget *parent);
    ~OrganizationTab();
};

class OrganizationTabEdit final : public AttributeEdit {
    Q_OBJECT

public:
    OrganizationTabEdit(Ui::OrganizationTab *ui, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;

private:
    Ui::OrganizationTab *ui;
    QStandardItemModel *reports_model;
};

#endif /* ORGANIZATION_TAB_H */
