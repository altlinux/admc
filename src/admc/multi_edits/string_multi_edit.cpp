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

#include "multi_edits/string_multi_edit.h"

#include "adldap.h"
#include "globals.h"

#include <QLineEdit>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QLabel>

StringMultiEdit::StringMultiEdit(const QString &attribute_arg, const QString &object_class_arg, QList<AttributeMultiEdit *> *edits_out, QObject *parent)
: AttributeMultiEdit(edits_out, parent)
{
    attribute = attribute_arg;
    object_class = object_class_arg;

    check = new QCheckBox();

    const QString label_text = g_adconfig->get_attribute_display_name(attribute, object_class) + ":";
    auto label = new QLabel(label_text);

    check_and_label_wrapper = new QWidget();
    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    check_and_label_wrapper->setLayout(layout);
    layout->addWidget(check);
    layout->addWidget(label);

    edit = new QLineEdit();

    connect(
        check, &QAbstractButton::toggled,
        this, &StringMultiEdit::on_check_toggled);
    connect(
        edit, &QLineEdit::textChanged,
        this, &StringMultiEdit::edited);
    on_check_toggled();
}

void StringMultiEdit::add_to_layout(QFormLayout *layout) {
    layout->addRow(check_and_label_wrapper, edit);
}

bool StringMultiEdit::apply(AdInterface &ad, const QList<QString> &target_list) {
    const bool need_to_apply = check->isChecked();
    if (!need_to_apply) {
        return true;
    }

    bool total_success = true;

    const QString new_value = edit->text();
    // TODO: return bool for total success?
    for (const QString &target : target_list) {
        const bool success = ad.attribute_replace_string(target, attribute, new_value);

        if (!success) {
            total_success = false;
        }
    }

    check->setChecked(false);

    return total_success;
}

void StringMultiEdit::reset() {
    check->setChecked(false);
}

void StringMultiEdit::on_check_toggled() {
    // Clear edit when check is unchecked
    if (check->isChecked()) {
        emit edited();
    } else {
        edit->setText(QString());
    }

    edit->setEnabled(check->isChecked());
}
