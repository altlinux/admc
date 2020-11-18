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

#ifndef MANAGER_EDIT_H
#define MANAGER_EDIT_H

#include "attribute_edit.h"

#include <QString>
#include <QMap>
#include <QList>

class QLineEdit;
class QPushButton;

class ManagerEdit final : public AttributeEdit {
Q_OBJECT
public:
    ManagerEdit(QObject *parent, QList<AttributeEdit *> *edits_out = nullptr);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private slots:
    void on_change();
    void on_details();
    void on_clear();

private:
    QLineEdit *edit;
    QPushButton *change_button;
    QPushButton *details_button;
    QPushButton *clear_button;

    QString current_value;

    void load_value(const QString &value);
};

#endif /* MANAGER_EDIT_H */
