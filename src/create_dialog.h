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

#ifndef CREATE_DIALOG_H
#define CREATE_DIALOG_H

#include "ad_interface.h"

#include <QString>
#include <QDialog>
#include <QList>

class QLineEdit;
class QVBoxLayout;
class QComboBox;

enum CreateType {
    CreateType_User,
    CreateType_Computer,
    CreateType_OU,
    CreateType_Group,
    CreateType_COUNT
};

// Create and open a create dialog appropriate for given type
void create_dialog(const QString &parent_dn, CreateType type, QWidget *parent);

class CreateDialog final : public QDialog {
Q_OBJECT

public:
    CreateDialog(const QString &parent_dn_arg, CreateType type_arg, QWidget *parent);

private slots:
    void on_accepted();

private:
    QString parent_dn;
    CreateType type;
    QLineEdit *name_edit;
};

class CreateGroupDialog final : public QDialog {
Q_OBJECT

public:
    CreateGroupDialog(const QString &parent_dn_arg, QWidget *parent);

private slots:
    void on_accepted();

private:
    QString parent_dn;
    QLineEdit *name_edit;
    QLineEdit *sama_edit;
    QComboBox *scope_combo;
    QComboBox *type_combo;
};

QString create_type_to_string(const CreateType &type);

#endif /* CREATE_DIALOG_H */
