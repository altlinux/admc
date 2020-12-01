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
#include <QFont>
#include <QFontDatabase>
#include <QDialogButtonBox>

// TODO: need to display value in these formats: hexadecimal, binary, decimal, octal. Currently only have hexadecimal.
// TODO: implement editing. Should editing be availabel though all display formats or just hex? Note that built-in qt input mask is very limited, and textedit doesn't even have one.

OctetEditDialog::OctetEditDialog(const QString attribute, const QList<QByteArray> values, QWidget *parent)
: EditDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Edit octet string"));

    QLabel *attribute_label = make_attribute_label(attribute);
    
    edit = new QLineEdit();
    edit->setReadOnly(true);
    const QFont fixed_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    edit->setFont(fixed_font);

    QDialogButtonBox *button_box = make_button_box(attribute);;

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(attribute_label);
    layout->addWidget(edit);
    layout->addWidget(button_box);

    if (!values.isEmpty()) {
        const QByteArray value = values[0];

        const QString display_value = octet_to_display_value(value);
        edit->setText(display_value);
    }
}

QList<QByteArray> OctetEditDialog::get_new_values() const {
    // TODO:
    return QList<QByteArray>();
}
