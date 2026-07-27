/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "attribute_dialogs/octet_attribute_dialog.h"
#include "attribute_dialogs/ui_octet_attribute_dialog.h"

#include "adldap.h"
#include "core/attribute.h"
#include "core/utils.h"
#include "core/globals.h"
#include "core/settings.h"
#include "utils.h"

#include <QFont>
#include <QFontDatabase>

#include <cstdint>
#include <cstdlib>
#include <QRegularExpression>

OctetDisplayFormat current_format(QComboBox *format_combo);

OctetAttributeDialog::OctetAttributeDialog(const QList<QByteArray> &value_list, const QString &attribute, const bool read_only, QWidget *parent)
: AttributeDialog(attribute, read_only, parent) {
    ui = new Ui::OctetAttributeDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    AttributeDialog::load_attribute_label(ui->attribute_label);

    prev_format = OctetDisplayFormat_Hexadecimal;

    const QFont fixed_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->edit->setFont(fixed_font);

    ui->edit->setReadOnly(read_only);

    const QByteArray value = value_list.value(0, QByteArray());
    const QString value_string = octet_bytes_to_string(value, current_format(ui->format_combo));
    ui->edit->setPlainText(value_string);

    settings_setup_dialog_geometry(SETTING_octet_attribute_dialog_geometry, this);

    connect(
        ui->format_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &OctetAttributeDialog::on_format_combo);
}

OctetAttributeDialog::~OctetAttributeDialog() {
    delete ui;
}

QList<QByteArray> OctetAttributeDialog::get_value_list() const {
    const QString text = ui->edit->toPlainText();

    if (!text.isEmpty()) {
        const QByteArray bytes = octet_string_to_bytes(text, current_format(ui->format_combo));

        return {bytes};
    } else {
        return {};
    }
}

void OctetAttributeDialog::accept() {
    const bool input_ok = check_input(current_format(ui->format_combo));

    if (input_ok) {
        AttributeDialog::accept();
    }
}

void OctetAttributeDialog::on_format_combo() {
    // Check that input is ok for previous format, otherwise
    // won't be able to convert it to new format
    const bool input_ok_for_prev_format = check_input(prev_format);
    if (input_ok_for_prev_format) {
        // Convert input in prev format back to bytes, then
        // convert bytes to input in new format
        // Ex: hex -> bytes -> octal
        const QString old_text = ui->edit->toPlainText();
        const QByteArray bytes = octet_string_to_bytes(old_text, prev_format);
        const QString new_text = octet_bytes_to_string(bytes, current_format(ui->format_combo));

        ui->edit->setPlainText(new_text);

        prev_format = current_format(ui->format_combo);
    } else {
        // Revert to previous format if input is invalid for
        // current format
        ui->format_combo->blockSignals(true);
        ui->format_combo->setCurrentIndex((int) prev_format);
        ui->format_combo->blockSignals(false);
    }
}

bool OctetAttributeDialog::check_input(const OctetDisplayFormat format) {
    const QString text = ui->edit->toPlainText();
    if (text.isEmpty()) {
        return true;
    }

    bool ok = attribute_validate_input(format, text);

    if (! ok) {
        const QString title = tr("Error");
        QString text;
        switch (format) {
        case OctetDisplayFormat_Hexadecimal:
            text = tr("Input must be strings of 2 hexadecimal digits separated by spaces. Example: \"0a 00 b5 ff\"");
            break;
        case OctetDisplayFormat_Binary:
            text = tr("Input must be strings of 8 binary digits separated by spaces. Example: \"01010010 01000010 01000010\"");
            break;
        case OctetDisplayFormat_Decimal:
            text = tr("Input must be strings of 3 decimal digits (0-255) separated by spaces. Example: \"010 000 191\"");
            break;
        case OctetDisplayFormat_Octal:
            text = tr("Input must be strings of 3 octal digits (0-377) separated by spaces.. Example: \"070 343 301\"");
            break;
        }

        message_box_warning(this, title, text);
    }

    return ok;
}

OctetDisplayFormat current_format(QComboBox *format_combo) {
    const int format_index = format_combo->currentIndex();
    const OctetDisplayFormat format = (OctetDisplayFormat) (format_index);

    return format;
}
