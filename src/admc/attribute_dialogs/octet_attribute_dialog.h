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

#ifndef OCTET_ATTRIBUTE_DIALOG_H
#define OCTET_ATTRIBUTE_DIALOG_H

#include "attribute_dialogs/attribute_dialog.h"

enum OctetDisplayFormat {
    OctetDisplayFormat_Hexadecimal = 0,
    OctetDisplayFormat_Binary,
    OctetDisplayFormat_Decimal,
    OctetDisplayFormat_Octal,
};

namespace Ui {
class OctetAttributeDialog;
}

class OctetAttributeDialog final : public AttributeDialog {
    Q_OBJECT

public:
    Ui::OctetAttributeDialog *ui;

    OctetAttributeDialog(const QList<QByteArray> &value_list, const QString &attribute, const bool read_only, QWidget *parent);
    ~OctetAttributeDialog();

    QList<QByteArray> get_value_list() const override;
    void accept() override;

private:
    OctetDisplayFormat prev_format;

    bool check_input(const OctetDisplayFormat format);
    void on_format_combo();
};

QString octet_bytes_to_string(const QByteArray bytes, const OctetDisplayFormat format);
QByteArray octet_string_to_bytes(const QString string, const OctetDisplayFormat format);

#endif /* OCTET_ATTRIBUTE_DIALOG_H */
