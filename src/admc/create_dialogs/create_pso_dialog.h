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

#ifndef CREATE_PSO_DIALOG_H
#define CREATE_PSO_DIALOG_H

#include "create_object_dialog.h"

namespace Ui {
class CreatePSODialog;
}

class PSOEditWidget;
class ProtectDeletionEdit;

class CreatePSODialog final : public CreateObjectDialog {

    Q_OBJECT

public:
    explicit CreatePSODialog(const QString &parent_dn_arg, QWidget *parent = nullptr);
    ~CreatePSODialog();

    void accept() override;
    QString get_created_dn() const override;

private:
    Ui::CreatePSODialog *ui;
    const QString parent_dn;
    ProtectDeletionEdit *deletion_edit;
};

#endif // CREATE_PSO_DIALOG_H
