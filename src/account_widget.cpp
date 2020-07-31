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

#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>

AccountWidget::AccountWidget(QWidget *parent)
: QWidget(parent)
{   
    const auto logon_name_label = new QLabel(tr("Logon name:"), this);

    logon_name_edit = new QLineEdit(this);

    disabled_check = new QCheckBox(tr("Account disabled"), this);

    password_expired_check = new QCheckBox(tr("User must change password on next logon"), this);

    // NOTE: can't show lock status, can only provide the button
    // determining whether an account is locked is VERY complicated
    const auto unlock_button = new QPushButton(tr("Unlock account"), this);

    const auto layout = new QGridLayout(this);
    layout->addWidget(logon_name_label, 0, 0);
    layout->addWidget(logon_name_edit, 1, 0);
    layout->addWidget(unlock_button, 2, 0);
    layout->addWidget(password_expired_check, 3, 0);
    layout->addWidget(disabled_check, 4, 0);

    connect(
        unlock_button, &QAbstractButton::clicked,
        this, &AccountWidget::on_unlock_button_clicked);

    auto connect_uac_check =
    [this](QCheckBox *check, int bit) {
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

    connect_uac_check(disabled_check, UAC_ACCOUNTDISABLE);
    connect_uac_check(password_expired_check, UAC_PASSWORD_EXPIRED);
}

void AccountWidget::change_target(const QString &dn) {
    target_dn = dn;

    const QString logon_name = AdInterface::instance()->attribute_get(target_dn, "userPrincipalName");
    logon_name_edit->setText(logon_name);

    const bool disabled = AdInterface::instance()->user_get_user_account_control(target_dn, UAC_ACCOUNTDISABLE);

    disabled_check->setChecked(disabled);
}

void AccountWidget::on_unlock_button_clicked() {
    AdInterface::instance()->attribute_replace(target_dn, "lockoutTime", "0");
}
