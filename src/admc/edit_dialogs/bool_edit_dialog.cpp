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

#include "edit_dialogs/bool_edit_dialog.h"
#include "ad_utils.h"
#include "ad_config.h"

#include <QRadioButton>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>

BoolEditDialog::BoolEditDialog(const QString attribute, const QList<QByteArray> values, QWidget *parent)
: EditDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("Edit boolean"));

    original_value = values;

    true_button = new QRadioButton(tr("True"));
    false_button = new QRadioButton(tr("False"));
    unset_button = new QRadioButton(tr("Unset"));

    auto button_box = new QDialogButtonBox();
    QPushButton *ok_button = button_box->addButton(QDialogButtonBox::Ok);
    QPushButton *reset_button = button_box->addButton(QDialogButtonBox::Reset);
    QPushButton *cancel_button = button_box->addButton(QDialogButtonBox::Cancel);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    add_attribute_label(top_layout, attribute);
    top_layout->addWidget(true_button);
    top_layout->addWidget(false_button);
    top_layout->addWidget(unset_button);
    top_layout->addWidget(button_box);

    if (ADCONFIG()->get_attribute_is_system_only(attribute)) {
        true_button->setEnabled(false);
        false_button->setEnabled(false);
        unset_button->setEnabled(false);
        button_box->setEnabled(false);
    }

    connect(
        ok_button, &QPushButton::clicked,
        this, &QDialog::accept);
    connect(
        reset_button, &QPushButton::clicked,
        this, &BoolEditDialog::reset);
    connect(
        cancel_button, &QPushButton::clicked,
        this, &BoolEditDialog::reject);

    reset();
}

void BoolEditDialog::reset() {
    if (original_value.isEmpty()) {
        unset_button->setChecked(true);
    } else {
        const QByteArray value_bytes = original_value[0];
        const QString value_string(value_bytes);
        const bool value = ad_string_to_bool(value_string);

        if (value) {
            true_button->setChecked(true);        
        } else {
            false_button->setChecked(true);
        }
    }
}

// TODO: this might be useful somewhere else
QList<QByteArray> BoolEditDialog::get_new_values() const {
    if (unset_button->isChecked()) {
        return QList<QByteArray>();
    } else if (true_button->isChecked()) {
        const QByteArray value = QString(LDAP_BOOL_TRUE).toUtf8();
        return {value};
    } else if (false_button->isChecked()) {
        const QByteArray value = QString(LDAP_BOOL_FALSE).toUtf8();
        return {value};
    } else {
        return QList<QByteArray>();
    }
}
