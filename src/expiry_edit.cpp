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

#include "expiry_edit.h"
#include "ad_interface.h"
#include "attribute_display_strings.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QGridLayout>
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

ExpiryEdit::ExpiryEdit() {
    never_check = new QCheckBox(tr("Never"));
    end_of_check = new QCheckBox(tr("End of:"));

    auto button_group = new QButtonGroup();
    button_group->setExclusive(true);
    button_group->addButton(never_check);
    button_group->addButton(end_of_check)edit_button = new QPushButton(tr("Edit expiry date"));
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

    QObject::connect(
        combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this]() {
            emit edited();
        });
}

void ExpiryEdit::load(const QString &dn) {
    const QString expiry_raw = AdInterface::instance()->attribute_get(target(), ATTRIBUTE_ACCOUNT_EXPIRES);
    original_value = expiry_raw;

    const bool never = datetime_is_never(ATTRIBUTE_ACCOUNT_EXPIRES, expiry_raw);

    // NOTE: prevent setChecked() from emitting signals
    QCheckBox *checks[] = {
        never_check,
        end_of_check
    };
    for (auto check : checks) {
        check->blockSignals(true);
    }
    {
        never_check->setChecked(never);
        end_of_check->setChecked(!never);
    }
    for (auto check : checks) {
        check->blockSignals(false);
    }

    display_label->setEnabled(!never);
    edit_button->setEnabled(!never);

    QString display_label_text;
    if (never) {
        // Load current date as default value when expiry is never
        const QDate default_expiry = QDate::currentDate();
        display_label_text = default_expiry.toString(DATE_FORMAT);
    } else {
        const QDateTime current_expiry = AdInterface::instance()->attribute_datetime_get(target(), ATTRIBUTE_ACCOUNT_EXPIRES);
        display_label_text = current_expiry.toString(DATE_FORMAT);
    }
    display_label->setText(display_label_text);


    original_value = current_code;

    const QVariant code_variant(current_code);
    const int index = combo->findData(code_variant);
    if (index != -1) {
        combo->setCurrentIndex(index);
    }

    emit edited();
}

void ExpiryEdit::add_to_layout(QGridLayout *layout) {
    const auto label = new QLabel(tr("Account expiry:"));

    layout->addWidget(label, layout->rowCount());
    layout->addWidget(never_check, layout->rowCount());
    layout->addWidget(end_of_check, layout->rowCount());
    layout->addWidget(display_label, layout->rowCount());
    layout->addWidget(edit_button, layout->rowCount());

    setup_edit_marker(this, label);
}

// TODO: limit date to the format's date range, make a f-n in adinterface that checks if format in limit
bool ExpiryEdit::verify_input(QWidget *parent) {
    return true;
}

bool ExpiryEdit::changed() const {
    const QString new_value = get_new_value();
    return (new_value != original_value);
}

bool ExpiryEdit::apply(const QString &dn) {
    const QString new_value = get_new_value();

    AdInterface::instance()->attribute_replace(target(), ATTRIBUTE_ACCOUNT_EXPIRES, new_value);
}

void AccountTab::on_never_check() {
    if (checkbox_is_checked(never_check)) {
        display_label->setEnabled(false);
        edit_button->setEnabled(false);

        emit edited();
    }
}

void AccountTab::on_end_of_check() {
    if (checkbox_is_checked(end_of_check)) {
        display_label->setEnabled(true);
        edit_button->setEnabled(true);

        emit edited();
    }
}

void AccountTab::on_edit_button() {
    auto dialog = new QDialog(this);

    auto label = new QLabel(tr("Edit expiry time"), dialog);
    auto calendar = new QCalendarWidget(dialog);
    auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok |  QDialogButtonBox::Cancel, dialog);

    const auto layout = new QVBoxLayout(dialog);
    layout->addWidget(label);
    layout->addWidget(calendar);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        [this, dialog, calendar]() {
            const QDate new_date = calendar->selectedDate();
            const QString new_date_string = new_date.toString(DATE_FORMAT);

            display_label->setText(new_date_string);

            dialog->accept();

            emit edited();
        });

    connect(
        button_box, &QDialogButtonBox::rejected,
        [dialog]() {
            dialog->reject();
        });

    dialog->open();
}

// NOTE: have to operate on raw string datetime values here because (never) value can't be expressed as QDateTime
QString ExpiryEdit::get_new_value() {
    const bool never = checkbox_is_checked(never_check);

    if (never) {
        return AD_LARGEINTEGERTIME_NEVER_1;
    } else {
        const QString new_date_string = display_label->text();
        const QDate new_date = QDate::fromString(new_date_string, DATE_FORMAT);
        const QDateTime new_datetime(new_date, END_OF_DAY);
        const QString value = datetime_to_string(ATTRIBUTE_ACCOUNT_EXPIRES, new_expiry);

        return value;
    }
}
