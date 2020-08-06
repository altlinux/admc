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
#include "ad_interface.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QButtonGroup>

// NOTE: https://ldapwiki.com/wiki/MMC%20Account%20Tab

bool checkbox_is_checked(const QCheckBox *checkbox) {
    return (checkbox->checkState() == Qt::Checked);
}

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

    expiry_edit = new QDateTimeEdit(this);

    const auto layout = new QVBoxLayout(this);
    layout->addWidget(logon_name_label);
    layout->addWidget(logon_name_edit);
    layout->addWidget(unlock_button);
    layout->addWidget(expiry_label);
    layout->addWidget(expiry_never_check);
    layout->addWidget(expiry_set_check);
    layout->addWidget(expiry_edit);

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
        expiry_edit, &QDateTimeEdit::dateTimeChanged,
        this, &AccountWidget::on_expiry_edit);

    // TODO:
    // "User cannot change password" - CAN'T just set PASSWD_CANT_CHANGE. See: https://docs.microsoft.com/en-us/windows/win32/adsi/modifying-user-cannot-change-password-ldap-provider?redirectedfrom=MSDN
    // "This account supports 128bit encryption" (and for 256bit)
    // "Use Kerberos DES encryption types for this account"
    const QList<int> uac_bits = {
        UAC_ACCOUNTDISABLE, UAC_PASSWORD_EXPIRED, UAC_DONT_EXPIRE_PASSWORD, UAC_USE_DES_KEY_ONLY, UAC_SMARTCARD_REQUIRED, UAC_NOT_DELEGATED, UAC_DONT_REQUIRE_PREAUTH
    };
    for (auto bit : uac_bits) {
        const QString text = get_uac_bit_description(bit);
        auto check = new QCheckBox(text);

        layout->addWidget(check);

        UACCheck uac_check = {
            check,
            bit
        };
        uac_checks.append(uac_check);

        connect(
            check, &QCheckBox::stateChanged,
            [this, check, bit]() {
                const bool checked = checkbox_is_checked(check);
                AdInterface::instance()->user_set_uac_bit(target_dn, bit, checked);
            });
    }
}

void AccountWidget::change_target(const QString &dn) {
    target_dn = dn;

    reset_logon_name_edit();

    // NOTE: block signals from setChecked()
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
        QCheckBox *check = uac_check.check;
        const int bit = uac_check.bit;

        const bool bit_is_set = AdInterface::instance()->user_get_uac_bit(target_dn, bit);

        check->setChecked(bit_is_set);
    }

    // NOTE: since each of the checkboxes makes a server modification, the whole widget is reloaded and what is below will always happen after checkbox state changes
    const bool expires_never = AdInterface::instance()->datetime_is_never(target_dn, ATTRIBUTE_ACCOUNT_EXPIRES);
    if (expires_never) {
        expiry_edit->setEnabled(false);
        expiry_never_check->setChecked(true);

        expiry_edit->setDateTime(QDateTime::currentDateTime());
    } else {
        expiry_edit->setEnabled(true);
        expiry_set_check->setChecked(true);
        
        const QDateTime expires = AdInterface::instance()->attribute_datetime_get(target_dn, ATTRIBUTE_ACCOUNT_EXPIRES);
        expiry_edit->setDateTime(expires);
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

void AccountWidget::on_expiry_set_check() {
    if (checkbox_is_checked(expiry_set_check)) {
        const QDateTime expires = expiry_edit->dateTime();
        
        // TODO: handle errors
        const AdResult result = AdInterface::instance()->attribute_datetime_replace(target_dn, ATTRIBUTE_ACCOUNT_EXPIRES, expires);
    }
}

void AccountWidget::on_expiry_edit() {
    if (checkbox_is_checked(expiry_set_check)) {
        const QDateTime new_datetime = expiry_edit->dateTime();

        // TODO: handle errors? don't know if possible
        const AdResult result = AdInterface::instance()->attribute_datetime_replace(target_dn, ATTRIBUTE_ACCOUNT_EXPIRES, new_datetime);
    }
}

void AccountWidget::reset_logon_name_edit() {
    const QString logon_name = AdInterface::instance()->attribute_get(target_dn, ATTRIBUTE_USER_PRINCIPAL_NAME);
    logon_name_edit->setText(logon_name);
}
