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

    disabled_label = new QLabel(this);
    disabled_check = new QCheckBox("Account disabled", this);

    // NOTE: can't show lock status, can only provide the button
    // determining whether an account is locked is VERY complicated
    unlock_button = new QPushButton("Unlock account", this);

    const auto layout = new QGridLayout(this);
    layout->addWidget(logon_name_label, 0, 0);
    layout->addWidget(logon_name_edit, 1, 0);
    layout->addWidget(disabled_label, 2, 0);
    layout->addWidget(disabled_check, 2, 0);
    layout->addWidget(unlock_button, 3, 0);

    connect(
        disabled_check, &QCheckBox::stateChanged,
        this, &AccountWidget::on_disabled_check_changed);

    connect(
        unlock_button, &QAbstractButton::clicked,
        this, &AccountWidget::on_unlock_button_clicked);
}

void AccountWidget::change_target(const QString &dn) {
    target_dn = dn;

    const QString logon_name = AdInterface::instance()->attribute_get(target_dn, "userPrincipalName");
    logon_name_edit->setText(logon_name);

    const bool disabled = AdInterface::instance()->user_get_user_account_control(target_dn, UAC_ACCOUNTDISABLE);

    disabled_check->setChecked(disabled);
}

void AccountWidget::on_disabled_check_changed() {
    const bool disabled_current = AdInterface::instance()->user_get_user_account_control(target_dn, UAC_ACCOUNTDISABLE);
    const bool disabled_new = (disabled_check->checkState() == Qt::Checked);

    if (disabled_current != disabled_new) {
        AdInterface::instance()->user_set_user_account_control(target_dn, UAC_ACCOUNTDISABLE, disabled_new);
    }
}

void AccountWidget::on_unlock_button_clicked() {
    AdInterface::instance()->attribute_replace(target_dn, "lockoutTime", "0");
}
