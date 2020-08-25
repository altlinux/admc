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

#include "account_tab.h"
#include "utils.h"
#include "attribute_edit.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QDateTime>
#include <QButtonGroup>
#include <QCalendarWidget>
#include <QDialog>

// TODO: logon hours, logon computers, remove crazy spacing

// NOTE: https://ldapwiki.com/wiki/MMC%20Account%20Tab

#define DATE_FORMAT "dd.MM.yyyy"

const QTime END_OF_DAY(23, 59);

QDateTime convert_to_end_of_day(const QDateTime &datetime);

AccountTab::AccountTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    const auto logon_name_edit = new StringEdit(ATTRIBUTE_USER_PRINCIPAL_NAME);
    edits.append(logon_name_edit);

    const auto unlock_button = new QPushButton(tr("Unlock account"));

    QList<AccountOption> options;
    for (int i = 0; i < AccountOption_COUNT; i++) {
        const AccountOption option = (AccountOption) i;
        options.append(option);
    }
    QMap<AccountOption, AccountOptionEdit *> option_edits = make_account_option_edits(options, this);
    for (auto edit : option_edits) {
        edits.append(edit);
    }

    auto edits_layout = new QGridLayout();
    for (auto edit : edits) {
        edit->add_to_layout(edits_layout);
    }

    connect_edits_to_tab(edits, this);

    auto expiry_label = new QLabel(tr("Account expiry:"));
    
    expiry_never_check = new QCheckBox(tr("Never"));
    expiry_set_check = new QCheckBox(tr("End of:"));
    auto expiry_button_group = new QButtonGroup();
    expiry_button_group->setExclusive(true);
    expiry_button_group->addButton(expiry_never_check);
    expiry_button_group->addButton(expiry_set_check);

    expiry_edit_button = new QPushButton(tr("Edit expiry date"));
    expiry_display = new QLabel();

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addLayout(edits_layout);

    // TODO: layout it out better, probably make a sub-layout so it's compacted
    top_layout->addWidget(expiry_label);
    top_layout->addWidget(expiry_never_check);
    top_layout->addWidget(expiry_set_check);
    top_layout->addWidget(expiry_display);
    top_layout->addWidget(expiry_edit_button);

    connect(
        unlock_button, &QAbstractButton::clicked,
        this, &AccountTab::on_unlock_button);
    connect(
        expiry_never_check, &QCheckBox::stateChanged,
        this, &AccountTab::on_expiry_never_check);
    connect(
        expiry_set_check, &QCheckBox::stateChanged,
        this, &AccountTab::on_expiry_set_check);
    connect(
        expiry_edit_button, &QAbstractButton::clicked,
        this, &AccountTab::on_expiry_edit_button);
}

void AccountTab::apply() {
    verify_and_apply_attribute_edits(edits, target(), this);

    // TODO: process errors
    // NOTE: have to operate on raw string datetime values here because (never) value can't be expressed as QDateTime
    AdResult expiry_result(false);
    const bool expiry_never = checkbox_is_checked(expiry_never_check);
    QString new_expiry_value;
    if (expiry_never) {
        new_expiry_value = AD_LARGEINTEGERTIME_NEVER_1;
    } else {
        const QString new_expiry_date_string = expiry_display->text();
        const QDate new_expiry_date = QDate::fromString(new_expiry_date_string, DATE_FORMAT);
        const QDateTime new_expiry(new_expiry_date, END_OF_DAY);
        new_expiry_value = datetime_to_string(ATTRIBUTE_ACCOUNT_EXPIRES, new_expiry);
    }

    const bool expiry_changed = (new_expiry_value != original_expiry_value);
    if (expiry_changed) {
        AdInterface::instance()->attribute_replace(target(), ATTRIBUTE_ACCOUNT_EXPIRES, new_expiry_value);
    }
}

void AccountTab::reload() {
    load_attribute_edits(edits, target());

    const QString expiry_raw = AdInterface::instance()->attribute_get(target(), ATTRIBUTE_ACCOUNT_EXPIRES);
    original_expiry_value = expiry_raw;
    const bool expiry_never = datetime_is_never(ATTRIBUTE_ACCOUNT_EXPIRES, expiry_raw);

    // NOTE: need to block signals from checks so that their slots aren't called
    QCheckBox *checks[] = {
        expiry_never_check,
        expiry_set_check
    };
    for (auto check : checks) {
        check->blockSignals(true);
    }
    {
        expiry_never_check->setChecked(expiry_never);
        expiry_set_check->setChecked(!expiry_never);
    }
    for (auto check : checks) {
        check->blockSignals(false);
    }

    expiry_display->setEnabled(!expiry_never);
    expiry_edit_button->setEnabled(!expiry_never);

    QString expiry_display_text;
    if (expiry_never) {
        // Load current date as default value when expiry is never
        const QDate default_expiry = QDate::currentDate();
        expiry_display_text = default_expiry.toString(DATE_FORMAT);
    } else {
        const QDateTime current_expiry = AdInterface::instance()->attribute_datetime_get(target(), ATTRIBUTE_ACCOUNT_EXPIRES);
        expiry_display_text = current_expiry.toString(DATE_FORMAT);
    }
    expiry_display->setText(expiry_display_text);
}

bool AccountTab::accepts_target() const {
    const bool is_user = AdInterface::instance()->is_user(target());

    return is_user;
}

void AccountTab::on_unlock_button() {
    AdInterface::instance()->user_unlock(target());
}

void AccountTab::on_expiry_never_check() {
    if (checkbox_is_checked(expiry_never_check)) {
        expiry_display->setEnabled(false);
        expiry_edit_button->setEnabled(false);

        on_edit_changed();
    }
}

// TODO: need time to be "end of" for that day, so 23:59?
void AccountTab::on_expiry_set_check() {
    if (checkbox_is_checked(expiry_set_check)) {
        expiry_display->setEnabled(true);
        expiry_edit_button->setEnabled(true);
        
        on_edit_changed();
    }
}

void AccountTab::on_expiry_edit_button() {
    auto dialog = new QDialog(this);

    auto label = new QLabel(tr("Edit expiry time"), dialog);
    auto calendar = new QCalendarWidget(dialog);
    const auto ok_button = new QPushButton(tr("OK"), dialog);
    const auto cancel_button = new QPushButton(tr("Cancel"), dialog);

    const auto layout = new QGridLayout(dialog);
    layout->addWidget(label, 0, 0);
    layout->addWidget(calendar, 1, 0);
    layout->addWidget(cancel_button, 2, 0, Qt::AlignLeft);
    layout->addWidget(ok_button, 2, 2, Qt::AlignRight);

    connect(
        ok_button, &QAbstractButton::clicked,
        [this, dialog, calendar]() {
            const QDate new_expiry_date = calendar->selectedDate();
            const QString new_expiry_date_string = new_expiry_date.toString(DATE_FORMAT);

            expiry_display->setText(new_expiry_date_string);

            on_edit_changed();

            dialog->accept();
        });

    connect(
        cancel_button, &QAbstractButton::clicked,
        [dialog]() {
            dialog->reject();
        });

    dialog->open();
}
