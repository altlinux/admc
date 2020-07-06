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

#include "user_widget.h"
#include "ad_interface.h"
#include "utils.h"
#include "password_dialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QInputDialog>

UserWidget::UserWidget(QWidget *parent)
: QWidget(parent)
{   
    const auto reset_password_button = new QPushButton("Reset password", this);

    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(reset_password_button);

    connect(
        reset_password_button, &QAbstractButton::clicked,
        this, &UserWidget::on_reset_password_button);
}

void UserWidget::change_target(const QString &dn) {
    target = dn;
}

void UserWidget::on_reset_password_button() {
    const auto password_dialog = new PasswordDialog(target, this);
    password_dialog->open();
}
