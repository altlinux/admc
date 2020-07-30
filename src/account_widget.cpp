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

    const auto layout = new QGridLayout(this);
    layout->addWidget(logon_name_label, 0, 0);
    layout->addWidget(logon_name_edit, 1, 0);
    layout->addWidget(disabled_label, 2, 0);
    layout->addWidget(disabled_check, 2, 0);

    connect(
        disabled_check, &QCheckBox::stateChanged,
        this, &AccountWidget::on_disabled_check_changed);
}

void AccountWidget::change_target(const QString &dn) {
    target_dn = dn;

    const QString logon_name = AdInterface::instance()->attribute_get(target_dn, "userPrincipalName");
    logon_name_edit->setText(logon_name);

    const bool enabled = AdInterface::instance()->user_enabled(target_dn);

    QString disabled_label_text;
    if (enabled) {
        disabled_label_text = "Account enabled";
    } else {
        disabled_label_text = "Account disabled";
    }

    disabled_check->setChecked(!enabled);
}

void AccountWidget::on_disabled_check_changed() {
    if (target_dn.isEmpty()) {
        return;
    }

    const bool disabled = (disabled_check->checkState() == Qt::Checked);

    if (disabled) {
        AdInterface::instance()->user_enable(target_dn);
    } else {
        AdInterface::instance()->user_disable(target_dn);
    }
}
