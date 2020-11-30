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

#include "edit_dialogs/octet_edit_dialog.h"

#include "ad_config.h"
#include "utils.h"
#include "attribute_display.h"

#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QFont>
#include <QFontDatabase>
#include <QDialogButtonBox>
#include <QPushButton>

// TODO: need to display value in these formats: hexadecimal, binary, decimal, octal. Currently only have hexadecimal.
// TODO: implement editing. Should editing be availabel though all display formats or just hex? Note that built-in qt input mask is very limited, and textedit doesn't even have one.

OctetEditDialog::OctetEditDialog(const QString attribute_arg, const QList<QByteArray> values, QWidget *parent)
: EditDialog(parent)
{
    original_values = values;
    attribute = attribute_arg;

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Edit octet string"));

    edit = new QLineEdit();
    edit->setReadOnly(true);

    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    auto reset_button = button_box->addButton(QDialogButtonBox::Reset);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    add_attribute_label(top_layout, attribute);
    top_layout->addWidget(edit);
    top_layout->addWidget(button_box);

    connect(
        ok_button, &QPushButton::clicked,
        this, &QDialog::accept);
    connect(
        reset_button, &QPushButton::clicked,
        this, &OctetEditDialog::reset);
    connect(
        cancel_button, &QAbstractButton::clicked,
        this, &OctetEditDialog::reject);

    reset();
}

QList<QByteArray> OctetEditDialog::get_new_values() const {
    return original_values;
}

void OctetEditDialog::reset() {
    if (!original_values.isEmpty()) {
        const QByteArray value = original_values[0];

        const QString display_value = octet_to_display_value(value);
        edit->setText(display_value);
    }
}
