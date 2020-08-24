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

    // expiry_never_check = new QCheckBox(tr("Never"), this);
    // expiry_set_check = new QCheckBox(tr("End of:"), this);
    // auto expiry_button_group = new QButtonGroup(this);
    // expiry_button_group->setExclusive(true);
    // expiry_button_group->addButton(expiry_never_check);
    // expiry_button_group->addButton(expiry_set_check);

    // auto expiry_label = new QLabel(tr("Account expires:"), this);

    // expiry_edit_button = new QPushButton(tr("Edit expiry date"), this);
    // expiry_display = new QLabel(this);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addLayout(edits_layout);

    connect(
        unlock_button, &QAbstractButton::clicked,
        this, &AccountTab::on_unlock_button);
    // connect(
    //     expiry_never_check, &QCheckBox::stateChanged,
    //     this, &AccountTab::on_expiry_never_check);
    // connect(
    //     expiry_set_check, &QCheckBox::stateChanged,
    //     this, &AccountTab::on_expiry_set_check);
    // connect(
    //     expiry_edit_button, &QAbstractButton::clicked,
    //     this, &AccountTab::on_expiry_edit_button);
}

void AccountTab::apply() {
    apply_attribute_edits(edits, target(), this);
}

void AccountTab::reload() {
    for (auto edit : edits) {
        edit->load(target());
    }

    // NOTE: block signals from setChecked() so they are not processed
    // as inputs
    // QList<QObject *> block_signals = {
    //     expiry_never_check, expiry_set_check
    // };
    // for (auto e : uac_checks) {
    //     block_signals.append(e.check);
    // }

    // for (auto e : block_signals) {
    //     e->blockSignals(true);
    // }

    // NOTE: since each of the checkboxes makes a server modification, the whole widget is reloaded and what is below will always happen after checkbox state changes
    // const QString expires_raw = AdInterface::instance()->attribute_get(target(), ATTRIBUTE_ACCOUNT_EXPIRES);
    // const bool expires_never = datetime_is_never(ATTRIBUTE_ACCOUNT_EXPIRES, expires_raw);
    // if (expires_never) {
    //     expiry_display->setEnabled(false);
    //     expiry_never_check->setChecked(true);
    //     expiry_edit_button->setEnabled(false);

    //     const QDate default_expiry = QDate::currentDate();
    //     const QString default_expiry_string = default_expiry.toString(DATE_FORMAT);
    //     expiry_display->setText(default_expiry_string);
    // } else {
    //     expiry_display->setEnabled(true);
    //     expiry_set_check->setChecked(true);
    //     expiry_edit_button->setEnabled(true);

    //     const QDateTime current_expiry = AdInterface::instance()->attribute_datetime_get(target(), ATTRIBUTE_ACCOUNT_EXPIRES);
    //     const QString current_expiry_string = current_expiry.toString(DATE_FORMAT);
    //     expiry_display->setText(current_expiry_string);
    // }

    // for (auto e : block_signals) {
    //     e->blockSignals(false);
    // }
}

bool AccountTab::accepts_target() const {
    const bool is_user = AdInterface::instance()->is_user(target());

    return is_user;
}

void AccountTab::on_unlock_button() {
    AdInterface::instance()->user_unlock(target());
}

void AccountTab::on_expiry_never_check() {
    // if (checkbox_is_checked(expiry_never_check)) {
    //     AdInterface::instance()->attribute_replace(target(), ATTRIBUTE_ACCOUNT_EXPIRES, AD_LARGEINTEGERTIME_NEVER_1);
    // }
}

// TODO: need time to be "end of" for that day, so 23:59?
void AccountTab::on_expiry_set_check() {
    // if (checkbox_is_checked(expiry_set_check)) {
    //     expiry_display->setEnabled(true);
    //     expiry_set_check->setChecked(true);
    //     expiry_edit_button->setEnabled(true);
    //     const QString expiry_string = expiry_display->text();
    //     const QDate expires_date = QDate::fromString(expiry_string, DATE_FORMAT);
    //     const QDateTime expires(expires_date, END_OF_DAY);

    //     // TODO: handle errors
    //     const AdResult result = AdInterface::instance()->attribute_datetime_replace(target(), ATTRIBUTE_ACCOUNT_EXPIRES, expires);
    // }
}

void AccountTab::on_expiry_edit_button() {
    // auto dialog = new QDialog(this);

    // auto label = new QLabel(tr("Edit expiry time"), dialog);
    // auto calendar = new QCalendarWidget(dialog);
    // const auto ok_button = new QPushButton(tr("OK"), dialog);
    // const auto cancel_button = new QPushButton(tr("Cancel"), dialog);

    // const auto layout = new QGridLayout(dialog);
    // layout->addWidget(label, 0, 0);
    // layout->addWidget(calendar, 1, 0);
    // layout->addWidget(cancel_button, 2, 0, Qt::AlignLeft);
    // layout->addWidget(ok_button, 2, 2, Qt::AlignRight);

    // connect(
    //     ok_button, &QAbstractButton::clicked,
    //     [this, dialog, calendar]() {
    //         const QDate date = calendar->selectedDate();
    //         const QDateTime new_expiry(date, END_OF_DAY);

    //         const AdResult result = AdInterface::instance()->attribute_datetime_replace(target(), ATTRIBUTE_ACCOUNT_EXPIRES, new_expiry);

    //         dialog->accept();
    //     });

    // connect(
    //     cancel_button, &QAbstractButton::clicked,
    //     [dialog]() {
    //         dialog->reject();
    //     });

    // dialog->open();
}
