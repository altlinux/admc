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

#include "edits/expiry_edit.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "ad_utils.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QDateTime>
#include <QButtonGroup>
#include <QCalendarWidget>
#include <QDialog>
#include <QDialogButtonBox>

#define DATE_FORMAT "dd.MM.yyyy"

const QTime END_OF_DAY(23, 59);

ExpiryEdit::ExpiryEdit(QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent)
{
    never_check = new QCheckBox(tr("Never"));
    end_of_check = new QCheckBox(tr("End of:"));

    auto button_group = new QButtonGroup();
    button_group->setExclusive(true);
    button_group->addButton(never_check);
    button_group->addButton(end_of_check);
    edit_button = new QPushButton(tr("Edit"));
    display_label = new QLabel();

    connect(
        never_check, &QCheckBox::stateChanged,
        this, &ExpiryEdit::on_never_check);
    connect(
        end_of_check, &QCheckBox::stateChanged,
        this, &ExpiryEdit::on_end_of_check);
    connect(
        edit_button, &QAbstractButton::clicked,
        this, &ExpiryEdit::on_edit_button);
}

void ExpiryEdit::load_internal(const AdObject &object) {
    const bool never =
    [object]() {
        const QString expiry_string = object.get_string(ATTRIBUTE_ACCOUNT_EXPIRES);
        return large_integer_datetime_is_never(expiry_string);
    }();

    never_check->setChecked(never);
    end_of_check->setChecked(!never);

    display_label->setEnabled(!never);
    edit_button->setEnabled(!never);

    QString display_label_text;
    if (never) {
        // Load current date as default value when expiry is never
        const QDate default_expiry = QDate::currentDate();
        display_label_text = default_expiry.toString(DATE_FORMAT);
    } else {
        const QDateTime datetime = object.get_datetime(ATTRIBUTE_ACCOUNT_EXPIRES);
        const QDateTime datetime_local = datetime.toLocalTime();

        display_label_text = datetime_local.toString(DATE_FORMAT);
    }
    display_label->setText(display_label_text);
}

void ExpiryEdit::set_read_only(const bool read_only) {
    never_check->setDisabled(read_only);
    end_of_check->setDisabled(read_only);
    edit_button->setDisabled(read_only);
}

void ExpiryEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = ADCONFIG()->get_attribute_display_name(ATTRIBUTE_ACCOUNT_EXPIRES, "") + ":";

    auto sublayout = new QVBoxLayout();
    sublayout->addWidget(never_check);
    sublayout->addWidget(end_of_check);
    sublayout->addWidget(display_label);
    sublayout->addWidget(edit_button);

    layout->addRow(label_text, sublayout);
}

bool ExpiryEdit::apply(const QString &dn) const {
    const bool never = never_check->isChecked();

    if (never) {
        return AD()->attribute_replace_string(dn, ATTRIBUTE_ACCOUNT_EXPIRES, AD_LARGE_INTEGER_DATETIME_NEVER_2);
    } else {
        const QString date_string = display_label->text();
        const QDate date = QDate::fromString(date_string, DATE_FORMAT);
        const QDateTime datetime(date, END_OF_DAY);

        return AD()->attribute_replace_datetime(dn, ATTRIBUTE_ACCOUNT_EXPIRES, datetime);
    }
}

void ExpiryEdit::on_never_check() {
    if (never_check->isChecked()) {
        display_label->setEnabled(false);
        edit_button->setEnabled(false);

        emit edited();
    }
}

void ExpiryEdit::on_end_of_check() {
    if (end_of_check->isChecked()) {
        display_label->setEnabled(true);
        edit_button->setEnabled(true);

        emit edited();
    }
}

void ExpiryEdit::on_edit_button() {
    auto dialog = new QDialog(edit_button);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    auto label = new QLabel(tr("Edit expiry time"), dialog);
    auto calendar = new QCalendarWidget(dialog);
    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);

    const auto layout = new QVBoxLayout(dialog);
    layout->addWidget(label);
    layout->addWidget(calendar);
    layout->addWidget(button_box);

    connect(
        ok_button, &QPushButton::clicked,
        [this, dialog, calendar]() {
            const QDate new_date = calendar->selectedDate();
            const QString new_date_string = new_date.toString(DATE_FORMAT);

            display_label->setText(new_date_string);

            dialog->accept();

            emit edited();
        });

    connect(
        cancel_button, &QPushButton::clicked,
        dialog, &QDialog::reject);

    dialog->open();
}
