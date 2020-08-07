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

#include "account_widget.h"

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
bool checkbox_is_checked(const QCheckBox *checkbox);

AccountWidget::AccountWidget(QWidget *parent)
: QWidget(parent)
{   
    const auto logon_name_label = new QLabel(tr("Logon name:"), this);

    logon_name_edit = new QLineEdit(this);

    const auto unlock_button = new QPushButton(tr("Unlock account"), this);

    expiry_never_check = new QCheckBox(tr("Never"), this);
    expiry_set_check = new QCheckBox(tr("End of:"), this);
    auto expiry_button_group = new QButtonGroup(this);
    expiry_button_group->setExclusive(true);
    expiry_button_group->addButton(expiry_never_check);
    expiry_button_group->addButton(expiry_set_check);

    auto expiry_label = new QLabel(tr("Account expires:"), this);

    expiry_edit_button = new QPushButton(tr("Edit expiry date"), this);
    expiry_display = new QLabel(this);

    const auto layout = new QVBoxLayout(this);
    layout->addWidget(logon_name_label);
    layout->addWidget(logon_name_edit);
    layout->addWidget(unlock_button);
    layout->addWidget(expiry_label);
    layout->addWidget(expiry_never_check);
    layout->addWidget(expiry_set_check);
    layout->addWidget(expiry_display);
    layout->addWidget(expiry_edit_button);

    connect(
        unlock_button, &QAbstractButton::clicked,
        this, &AccountWidget::on_unlock_button);
    connect(
        logon_name_edit, &QLineEdit::editingFinished,
        this, &AccountWidget::on_logon_name_edit);
    connect(
        expiry_never_check, &QCheckBox::stateChanged,
        this, &AccountWidget::on_expiry_never_check);
    connect(
        expiry_set_check, &QCheckBox::stateChanged,
        this, &AccountWidget::on_expiry_set_check);
    connect(
        expiry_edit_button, &QAbstractButton::clicked,
        this, &AccountWidget::on_expiry_edit_button);

    for (int i = 0; i < AccountOption_COUNT; i++) {
        const AccountOption option = (AccountOption) i;
        const QString text = get_account_option_description(option);
        auto check = new QCheckBox(text);

        layout->addWidget(check);

        UACCheck uac_check = {
            check,
            option
        };
        uac_checks.append(uac_check);

        connect(
            check, &QCheckBox::stateChanged,
            [this, check, option]() {
                const bool checked = checkbox_is_checked(check);
                AdInterface::instance()->user_set_account_option(target_dn, option, checked);
            });
    }
}

void AccountWidget::change_target(const QString &dn) {
    target_dn = dn;

    reset_logon_name_edit();

    // NOTE: block signals from setChecked() so they are not processed
    // as inputs
    QList<QObject *> block_signals = {
        expiry_never_check, expiry_set_check
    };
    for (auto e : uac_checks) {
        block_signals.append(e.check);
    }

    for (auto e : block_signals) {
        e->blockSignals(true);
    }

    for (auto uac_check : uac_checks) {
        const bool option_is_set = AdInterface::instance()->user_get_account_option(target_dn, uac_check.option);

        uac_check.check->setChecked(option_is_set);
    }

    // NOTE: since each of the checkboxes makes a server modification, the whole widget is reloaded and what is below will always happen after checkbox state changes
    const QString expires_raw = AdInterface::instance()->attribute_get(target_dn, ATTRIBUTE_ACCOUNT_EXPIRES);
    const bool expires_never = datetime_is_never(ATTRIBUTE_ACCOUNT_EXPIRES, expires_raw);
    if (expires_never) {
        expiry_display->setEnabled(false);
        expiry_never_check->setChecked(true);
        expiry_edit_button->setEnabled(false);

        const QDate default_expiry = QDate::currentDate();
        const QString default_expiry_string = default_expiry.toString(DATE_FORMAT);
        expiry_display->setText(default_expiry_string);
    } else {
        expiry_display->setEnabled(true);
        expiry_set_check->setChecked(true);
        expiry_edit_button->setEnabled(true);

        const QDateTime current_expiry = AdInterface::instance()->attribute_datetime_get(target_dn, ATTRIBUTE_ACCOUNT_EXPIRES);
        const QString current_expiry_string = current_expiry.toString(DATE_FORMAT);
        expiry_display->setText(current_expiry_string);
    }

    for (auto e : block_signals) {
        e->blockSignals(false);
    }
}

void AccountWidget::on_unlock_button() {
    AdInterface::instance()->user_unlock(target_dn);
}

void AccountWidget::on_logon_name_edit() {
    const QString new_logon_name = logon_name_edit->text();
    const QString current_logon_name = AdInterface::instance()->attribute_get(target_dn, ATTRIBUTE_USER_PRINCIPAL_NAME);

    if (new_logon_name != current_logon_name) {
        const AdResult result = AdInterface::instance()->attribute_replace(target_dn, ATTRIBUTE_USER_PRINCIPAL_NAME, new_logon_name);

        if (!result.success) {
            // TODO: show error
            reset_logon_name_edit();
        }
    }
}

void AccountWidget::on_expiry_never_check() {
    if (checkbox_is_checked(expiry_never_check)) {
        AdInterface::instance()->attribute_replace(target_dn, ATTRIBUTE_ACCOUNT_EXPIRES, AD_LARGEINTEGERTIME_NEVER_1);
    }
}

// TODO: need time to be "end of" for that day, so 23:59?
void AccountWidget::on_expiry_set_check() {
    if (checkbox_is_checked(expiry_set_check)) {
        expiry_display->setEnabled(true);
        expiry_set_check->setChecked(true);
        expiry_edit_button->setEnabled(true);
        const QString expiry_string = expiry_display->text();
        const QDate expires_date = QDate::fromString(expiry_string, DATE_FORMAT);
        const QDateTime expires(expires_date, END_OF_DAY);

        // TODO: handle errors
        const AdResult result = AdInterface::instance()->attribute_datetime_replace(target_dn, ATTRIBUTE_ACCOUNT_EXPIRES, expires);
    }
}

void AccountWidget::on_expiry_edit_button() {
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
            const QDate date = calendar->selectedDate();
            const QDateTime new_expiry(date, END_OF_DAY);

            const AdResult result = AdInterface::instance()->attribute_datetime_replace(target_dn, ATTRIBUTE_ACCOUNT_EXPIRES, new_expiry);

            dialog->accept();
        });

    connect(
        cancel_button, &QAbstractButton::clicked,
        [dialog]() {
            dialog->reject();
        });

    dialog->open();
}

void AccountWidget::reset_logon_name_edit() {
    const QString logon_name = AdInterface::instance()->attribute_get(target_dn, ATTRIBUTE_USER_PRINCIPAL_NAME);
    logon_name_edit->setText(logon_name);
}

bool checkbox_is_checked(const QCheckBox *checkbox) {
    return (checkbox->checkState() == Qt::Checked);
}
