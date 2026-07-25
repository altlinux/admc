/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2025-2026 BaseALT Ltd.
 * Copyright (C) 2025 Semyon Knyazev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#ifndef CREATE_SUBNET_DIALOG_H
#define CREATE_SUBNET_DIALOG_H

#include "create_dialogs/create_object_dialog.h"

namespace Ui {
class CreateSubnetDialog;
}

class AdInterface;

class CreateSubnetDialog final : public CreateObjectDialog {
    Q_OBJECT

public:
    explicit CreateSubnetDialog(AdInterface &ad, const QString &parent_dn_arg, QWidget *parent = nullptr);
    ~CreateSubnetDialog();

    void accept() override;
    QString get_created_dn() const override;

private:
    Ui::CreateSubnetDialog *ui;
    QString parent_dn;
    QString created_dn;


    void check_prefix_validity(const QString &address);
    bool validate_ipv4_prefix(quint32 ip, int prefix);
    bool validate_ipv6_prefix(const quint8 ipv6[16], int prefix);
};

#endif // CREATE_SUBNET_DIALOG_H
