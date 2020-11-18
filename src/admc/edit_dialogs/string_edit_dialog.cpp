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

#include "string_edit_dialog.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "edits/attribute_edit.h"
#include "edits/string_edit.h"
#include "status.h"
#include "utils.h"
#include "config.h"

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

// TODO: figure out what can and can't be renamed and disable renaming for exceptions (computers can't for example)

StringEditDialog::StringEditDialog(const QString attribute, const QList<QByteArray> values)
: EditDialog()
{
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(QString(tr("Edit %1 - %2")).arg(attribute, ADMC_APPLICATION_NAME));

    edit = new QLineEdit();

    if (ADCONFIG()->attribute_is_number(attribute)) {
        set_line_edit_to_numbers_only(edit);
    }

    const QByteArray value = values.value(0, QByteArray());
    original_value = QString::fromUtf8(value);
    edit->setText(original_value);

    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    auto reset_button = button_box->addButton(QDialogButtonBox::Reset);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(edit);
    top_layout->addWidget(button_box);

    if (ADCONFIG()->get_attribute_is_system_only(attribute)) {
        edit->setReadOnly(true);
        button_box->setEnabled(false);
    }

    connect(
        ok_button, &QPushButton::clicked,
        this, &QDialog::accept);
    connect(
        reset_button, &QPushButton::clicked,
        this, &StringEditDialog::reset);
    connect(
        cancel_button, &QPushButton::clicked,
        this, &StringEditDialog::reject);
}

void StringEditDialog::reset() {
    edit->setText(original_value);
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
