/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2023 BaseALT Ltd.
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

#ifndef HEX_NUMBER_ATTRIBUTE_DIALOG_H
#define HEX_NUMBER_ATTRIBUTE_DIALOG_H

#include "attribute_dialogs/attribute_dialog.h"

namespace Ui {
class HexNumberAttributeDialog;
}

class HexNumberAttributeDialog final : public AttributeDialog {
    Q_OBJECT

public:
    HexNumberAttributeDialog(const QList<QByteArray> &value_list, const QString &attribute, const bool read_only, QWidget *parent);
    ~HexNumberAttributeDialog();

    QList<QByteArray> get_value_list() const override;

private:
    Ui::HexNumberAttributeDialog *ui;
};

#endif // HEX_NUMBER_ATTRIBUTE_DIALOG_H
