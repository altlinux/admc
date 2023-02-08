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

#include "attribute_dialogs/hex_number_attribute_dialog.h"
#include "attribute_dialogs/ui_hex_number_attribute_dialog.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"
#include "settings.h"
#include <QDebug>

HexNumberAttributeDialog::HexNumberAttributeDialog(const QList<QByteArray> &value_list, const QString &attribute, const bool read_only, QWidget *parent)
: AttributeDialog(attribute, read_only, parent), ui(new Ui::HexNumberAttributeDialog) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    AttributeDialog::load_attribute_label(ui->attribute_label);

    const QByteArray value = value_list.value(0, QByteArray());

    set_line_edit_to_hex_numbers_only(ui->edit);

    ui->edit->setMaxLength(2 * sizeof(int));
    ui->edit->setReadOnly(read_only);

    //quint32 conversion for unsigned hex representation
    QString value_text = QString::number((quint32)value.toInt(), 16);
    ui->edit->setText(value_text);

    settings_setup_dialog_geometry(SETTING_hex_number_attribute_dialog_geometry, this);
}

HexNumberAttributeDialog::~HexNumberAttributeDialog() {
    delete ui;
}

QList<QByteArray> HexNumberAttributeDialog::get_value_list() const {
    const QString new_value_string = ui->edit->text();
    bool ok_toUInt;
    int int_value = (int)new_value_string.toUInt(&ok_toUInt, 16);

    if (new_value_string.isEmpty() || !ok_toUInt) {
        return {};
    } else {
        const QByteArray new_value = QByteArray::number(int_value);
        return {new_value};
    }
}
