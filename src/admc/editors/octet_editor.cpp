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

#include "editors/octet_editor.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFont>
#include <QFontDatabase>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include <cstdint>
#include <cstdlib>

OctetDisplayFormat current_format(QComboBox *format_combo);
int format_base(const OctetDisplayFormat format);
char *itoa(int value, char *result, int base);

OctetEditor::OctetEditor(const QString attribute, const QList<QByteArray> values, QWidget *parent)
: AttributeEditor(parent) {
    setWindowTitle(tr("Edit octet string"));

    prev_format = OctetDisplayFormat_Hexadecimal;

    QLabel *attribute_label = make_attribute_label(attribute);

    format_combo = new QComboBox();
    format_combo->addItem(tr("Hexadecimal"), (int) OctetDisplayFormat_Hexadecimal);
    format_combo->addItem(tr("Binary"), (int) OctetDisplayFormat_Binary);
    format_combo->addItem(tr("Decimal"), (int) OctetDisplayFormat_Decimal);
    format_combo->addItem(tr("Octal"), (int) OctetDisplayFormat_Octal);

    edit = new QPlainTextEdit();
    const QFont fixed_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    edit->setFont(fixed_font);

    const QByteArray value = values.value(0, QByteArray());
    const QString value_string = octet_bytes_to_string(value, current_format(format_combo));
    edit->setPlainText(value_string);

    if (g_adconfig->get_attribute_is_system_only(attribute)) {
        edit->setReadOnly(true);
    }

    QDialogButtonBox *button_box = make_button_box(attribute);
    ;

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(attribute_label);
    layout->addWidget(format_combo);
    layout->addWidget(edit);
    layout->addWidget(button_box);

    connect(
        format_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &OctetEditor::on_format_combo);
}

QList<QByteArray> OctetEditor::get_new_values() const {
    const QString text = edit->toPlainText();
    const QByteArray bytes = octet_string_to_bytes(text, current_format(format_combo));

    return {bytes};
}

void OctetEditor::accept() {
    const bool input_ok = check_input(current_format(format_combo));

    if (input_ok) {
        QDialog::accept();
    }
}

void OctetEditor::on_format_combo() {
    static int prev_index = 0;

    // Check that input is ok for previous format, otherwise
    // won't be able to convert it to new format
    const bool input_ok_for_prev_format = check_input(prev_format);
    if (input_ok_for_prev_format) {
        // Convert input in prev format back to bytes, then
        // convert bytes to input in new format
        // Ex: hex -> bytes -> octal
        const QString old_text = edit->toPlainText();
        const QByteArray bytes = octet_string_to_bytes(old_text, prev_format);
        const QString new_text = octet_bytes_to_string(bytes, current_format(format_combo));

        edit->setPlainText(new_text);

        prev_format = current_format(format_combo);
    } else {
        // Revert to previous format if input is invalid for
        // current format
        format_combo->blockSignals(true);
        format_combo->setCurrentIndex(prev_index);
        format_combo->blockSignals(false);
    }
}

bool OctetEditor::check_input(const OctetDisplayFormat format) {
    const bool ok = [=]() {
        const QString text = edit->toPlainText();

        if (text.isEmpty()) {
            return true;
        }

        const QList<QString> text_split = text.split(" ");

        // Check that all elements of text (separated by
        // space) match the format
        for (const QString &element : text_split) {
            switch (format) {
                case OctetDisplayFormat_Hexadecimal: {
                    const QRegExp rx("([0-9a-f]{2})");

                    if (!rx.exactMatch(element)) {
                        return false;
                    }

                    break;
                }
                case OctetDisplayFormat_Binary: {
                    const QRegExp rx("([0-1]{8})");
                    if (!rx.exactMatch(element)) {
                        return false;
                    }

                    break;
                }
                case OctetDisplayFormat_Decimal: {
                    const QRegExp rx("([0-9]{3})");
                    if (!rx.exactMatch(element)) {
                        return false;
                    }

                    const int number = element.toInt();
                    if (0 > number || number > 255) {
                        return false;
                    }

                    break;
                }
                case OctetDisplayFormat_Octal: {
                    const QRegExp rx("([0-7]{3})");
                    if (!rx.exactMatch(element)) {
                        return false;
                    }

                    // NOTE: technically forcing an octal
                    // number into decimal here, but it
                    // works out fine for range checking
                    const int number = element.toInt();
                    if (0 > number || number > 377) {
                        return false;
                    }

                    break;
                }
            }
        }

        return true;
    }();

    if (!ok) {
        const QString title = tr("Error");

        const QString text = [format]() {
            switch (format) {
                case OctetDisplayFormat_Hexadecimal: return tr("Input must be strings of 2 hexadecimal digits separated by spaces. Example: \"0a 00 b5 ff\"");
                case OctetDisplayFormat_Binary: return tr("Input must be strings of 8 binary digits separated by spaces. Example: \"01010010 01000010 01000010\"");
                case OctetDisplayFormat_Decimal: return tr("Input must be strings of 3 decimal digits (0-255) separated by spaces. Example: \"010 000 191\"");
                case OctetDisplayFormat_Octal: return tr("Input must be strings of 3 octal digits (0-377) separated by spaces.. Example: \"070 343 301\"");
            }
            return QString();
        }();

        message_box_warning(this, title, text);
    }

    return ok;
}

OctetDisplayFormat current_format(QComboBox *format_combo) {
    const int format_index = format_combo->currentIndex();
    const QVariant format_variant = format_combo->itemData(format_index);
    const OctetDisplayFormat format = (OctetDisplayFormat)(format_variant.toInt());

    return format;
}

QString octet_bytes_to_string(const QByteArray bytes, const OctetDisplayFormat format) {
    QString out;

    for (int i = 0; i < bytes.size(); i++) {
        if (i > 0) {
            out += " ";
        }

        const char byte_char = bytes[i];
        uint8_t byte = (uint8_t) byte_char;

        char buffer[100];

        const int base = format_base(format);

        itoa((int) byte, buffer, base);

        const QString byte_string_unpadded(buffer);

        const int string_length = [format]() {
            switch (format) {
                case OctetDisplayFormat_Hexadecimal: return 2;
                case OctetDisplayFormat_Binary: return 8;
                case OctetDisplayFormat_Decimal: return 3;
                case OctetDisplayFormat_Octal: return 3;
            }
            return 0;
        }();

        // "5" => "005"
        // "f" => "0f"
        const QString byte_string = byte_string_unpadded.rightJustified(string_length, '0');

        out += byte_string;
    }

    return out;
}

QByteArray octet_string_to_bytes(const QString string, const OctetDisplayFormat format) {
    if (string.isEmpty()) {
        return QByteArray();
    }

    const QList<QString> string_split = string.split(" ");

    QByteArray out;

    for (const QString &byte_string_padded : string_split) {
        // NOTE: remove padding because strtol doesn't understand it
        // "005" => "5"
        const QString byte_string = [byte_string_padded]() {
            QString byte = byte_string_padded;
            while (byte[0] == '0' && byte.size() > 0) {
                byte.remove(0, 1);
            }
            return byte;
        }();

        const QByteArray byte_bytes = byte_string.toLocal8Bit();
        const char *byte_cstr = byte_bytes.constData();
        const int base = format_base(format);
        const long int byte_li = strtol(byte_cstr, NULL, base);
        const char byte = (char) byte_li;

        out.append(byte);
    }

    return out;
}

int format_base(const OctetDisplayFormat format) {
    switch (format) {
        case OctetDisplayFormat_Hexadecimal: return 16;
        case OctetDisplayFormat_Binary: return 2;
        case OctetDisplayFormat_Decimal: return 10;
        case OctetDisplayFormat_Octal: return 8;
    }
    return 0;
}

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
// NOTE: not included in base lib, so had to copypaste. Maybe find some other more popular implementation and use that (with appropriate license). Preferrably something that automatically pads the result (leading 0's).
char *itoa(int value, char *result, int base) {
    // check that the base is valid
    if (base < 2 || base > 36) {
        *result = '\0';
        return result;
    }

    char *ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];
    } while (value);

    // Apply negative sign
    if (tmp_value < 0)
        *ptr++ = '-';
    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}
