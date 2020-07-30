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

AccountWidget::AccountWidget(QWidget *parent)
: QWidget(parent)
{   
    const auto logon_name_label = new QLabel(tr("Logon name:"), this);

    logon_name_edit = new QLineEdit(this);

    lock_label = new QLabel(this);
    lock_button = new QPushButton(this);

    const auto layout = new QGridLayout(this);
    layout->addWidget(logon_name_label, 0, 0);
    layout->addWidget(logon_name_edit, 1, 0);
    layout->addWidget(lock_label, 2, 0);
    layout->addWidget(lock_button, 2, 1);

    connect(
        lock_button, &QAbstractButton::clicked,
        this, &AccountWidget::on_lock_button);
}

void AccountWidget::change_target(const QString &dn) {
    target_dn = dn;

    const QString logon_name = AdInterface::instance()->attribute_get(target_dn, "userPrincipalName");
    logon_name_edit->setText(logon_name);

    const bool locked = AdInterface::instance()->user_locked(target_dn);

    QString lock_label_text;
    if (locked) {
        lock_label_text = "Account locked";
    } else {
        lock_label_text = "Account unlocked";
    }
    lock_label->setText(lock_label_text);

    QString lock_button_text;
    if (locked) {
        lock_button_text = "Unlock";
    } else {
        lock_button_text = "Lock";
    }
    lock_button->setText(lock_button_text);
}

void AccountWidget::on_lock_button() {
    const bool locked = AdInterface::instance()->user_locked(target_dn);
   
    if (locked) {
        AdInterface::instance()->user_unlock(target_dn);
    } else {
        AdInterface::instance()->user_lock(target_dn);
    }
}
