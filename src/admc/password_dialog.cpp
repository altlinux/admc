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

#include "password_dialog.h"
#include "ad_interface.h"
#include "edits/password_edit.h"
#include "edits/unlock_edit.h"
#include "edits/account_option_edit.h"
#include "status.h"
#include "utils.h"
#include "config.h"

#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>

PasswordDialog::PasswordDialog(const QString &target_arg)
: QDialog()
{
    target = target_arg;
    const AdObject object = AD()->search_object(target);

    setAttribute(Qt::WA_DeleteOnClose);

    const QString name = dn_get_name(target);
    const QString title = QString(tr("Change password of \"%1\" - %2")).arg(name, ADMC_APPLICATION_NAME);
    setWindowTitle(title);

    pass_edit = new PasswordEdit(this, &edits);
    new AccountOptionEdit(AccountOption_PasswordExpired, this, &edits);
    new UnlockEdit(this, &edits);

    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);

    auto edits_layout = new QFormLayout();
    edits_add_to_layout(edits, edits_layout);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(edits_layout);
    layout->addWidget(button_box);

    edits_load(edits, object);

    connect(
        ok_button, &QPushButton::clicked,
        this, &PasswordDialog::accept);
    connect(
        cancel_button, &QPushButton::clicked,
        this, &QDialog::reject);
}

void PasswordDialog::accept() {
    const bool pass_confirmed = pass_edit->check_confirm();

    if (pass_confirmed) {
        const int errors_index = Status::instance()->get_errors_size();

        edits_apply(edits, target);

        QDialog::close();

        Status::instance()->show_errors_popup(errors_index);
    }
}
