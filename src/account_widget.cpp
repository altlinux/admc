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

// LOCKOUT - set by system, unlocked by setting lockoutTime, but the bit doesn't change until user logs in, therefore very difficult to show whether user is locked or not

// PASSWD_CANT_CHANGE - difficult, see: https://docs.microsoft.com/en-us/windows/win32/adsi/modifying-user-cannot-change-password-ldap-provider?redirectedfrom=MSDN

AccountWidget::AccountWidget(QWidget *parent)
: QWidget(parent)
{   
    const auto logon_name_label = new QLabel(tr("Logon name:"), this);

    logon_name_edit = new QLineEdit(this);

    const auto unlock_button = new QPushButton(tr("Unlock account"), this);

    const auto layout = new QVBoxLayout(this);
    layout->addWidget(logon_name_label);
    layout->addWidget(logon_name_edit);
    layout->addWidget(unlock_button);

    connect(
        unlock_button, &QAbstractButton::clicked,
        this, &AccountWidget::on_unlock_button_clicked);

    auto connect_uac_check =
    [this, layout](const QString &text, int bit) {
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
                const bool current_state = AdInterface::instance()->user_get_user_account_control(target_dn, bit);
                const bool new_state = (check->checkState() == Qt::Checked);

                if (current_state != new_state) {
                    AdInterface::instance()->user_set_user_account_control(target_dn, bit, new_state);
                }
            });
    };

    connect_uac_check(tr("Account disabled"), UAC_ACCOUNTDISABLE);
    connect_uac_check(tr("User must change password on next logon"), UAC_PASSWORD_EXPIRED);
    connect_uac_check(tr("Don't expire password"), DONT_EXPIRE_PASSWORD);
}

void AccountWidget::change_target(const QString &dn) {
    target_dn = dn;

    const QString logon_name = AdInterface::instance()->attribute_get(target_dn, "userPrincipalName");
    logon_name_edit->setText(logon_name);

    for (auto uac_check : uac_checks) {
        QCheckBox *check = uac_check.check;
        const int bit = uac_check.bit;

        const bool bit_is_set = AdInterface::instance()->user_get_user_account_control(target_dn, bit);

        check->setChecked(bit_is_set);
    }
}

void AccountWidget::on_unlock_button_clicked() {
    AdInterface::instance()->attribute_replace(target_dn, "lockoutTime", "0");
}
