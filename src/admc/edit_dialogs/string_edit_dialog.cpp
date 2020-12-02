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

#include "edit_dialogs/string_edit_dialog.h"
#include "ad_config.h"
#include "utils.h"

#include <QLineEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>

StringEditDialog::StringEditDialog(const QString attribute, const QList<QByteArray> values, QWidget *parent)
: EditDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    const QString title =
    [attribute]() {
        const AttributeType type = ADCONFIG()->get_attribute_type(attribute);

        switch (type) {
            case AttributeType_Integer: return tr("Edit integer");
            case AttributeType_LargeInteger: return tr("Edit large integer");
            case AttributeType_Enumeration: return tr("Edit enumeration");
            default: break;
        };

        return tr("Edit string");
    }();
    setWindowTitle(title);

    QLabel *attribute_label = make_attribute_label(attribute);

    edit = new QLineEdit();

    if (ADCONFIG()->attribute_is_number(attribute)) {
        set_line_edit_to_numbers_only(edit);
    }

    ADCONFIG()->limit_edit(edit, attribute);

    const QByteArray value = values.value(0, QByteArray());
    const QString value_string = QString(value);
    edit->setText(value_string);

    QDialogButtonBox *button_box = make_button_box(attribute);;

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(attribute_label);
    layout->addWidget(edit);
    layout->addWidget(button_box);

    const bool system_only = ADCONFIG()->get_attribute_is_system_only(attribute);
    if (system_only) {
        edit->setReadOnly(true);
    }
}

QList<QByteArray> StringEditDialog::get_new_values() const {
    const QString new_value_string = edit->text();

    if (new_value_string.isEmpty()) {
        return {};
    } else {
        const QByteArray new_value = new_value_string.toUtf8();
        return {new_value};
    }
}
