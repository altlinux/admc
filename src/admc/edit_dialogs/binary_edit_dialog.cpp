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

#include "binary_edit_dialog.h"
#include "ad_config.h"
#include "utils.h"
#include "attribute_display.h"

#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QFont>
#include <QFontDatabase>

BinaryEditDialog::BinaryEditDialog(const QString attribute, const QList<QByteArray> values)
: EditDialog()
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(300, 300);

    original_values = values;

    const auto title_text = QString(tr("Attribute: %1")).arg(attribute);
    const auto title_label = new QLabel(title_text);

    string_display = new QLineEdit();
    string_display->setReadOnly(true);

    auto hex_display_label = new QLabel(tr("Hexadecimal:"));
    auto hex_display = new QTextEdit();
    hex_display->setReadOnly(true);
    const QFont fixed_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    hex_display->setCurrentFont(fixed_font);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(title_label);
    top_layout->addWidget(string_display);
    top_layout->addWidget(hex_display);

    // Load value into widgets
    if (!values.isEmpty()) {
        const QByteArray value = values[0];

        const QString display_value = attribute_display_value(attribute, value);
        string_display->setText(display_value);

        const QByteArray value_hex = value.toHex();
        QString value_hex_string = QString(value_hex);
        // Separate bytes by spaces
        for (int i = value_hex_string.size() - 2; i > 0; i-=2) {
            value_hex_string.insert(i, " ");
        }
        hex_display->setText(value_hex_string);
    }
}

QList<QByteArray> BinaryEditDialog::get_new_values() const {
    return original_values;
}
