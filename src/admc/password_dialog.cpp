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
#include "ad/ad_interface.h"
#include "ad/ad_utils.h"
#include "ad/ad_object.h"
#include "edits/password_edit.h"
#include "edits/unlock_edit.h"
#include "edits/account_option_edit.h"
#include "status.h"
#include "utils.h"

#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>

PasswordDialog::PasswordDialog(const QList<QString> &targets, QWidget *parent)
: QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    if (targets.size() != 1) {
        close();

        return;
    }
    
    target = targets[0];

    AdInterface ad;
    if (ad_failed(ad)) {
        close();
        return;
    }

    const AdObject object = ad.search_object(target);

    const QString name = dn_get_name(target);
    const QString title = QString(tr("Change password of object \"%1\"")).arg(name);
    setWindowTitle(title);

    new PasswordEdit(&edits, this);
    
    auto pass_expired_check = new AccountOptionEdit(AccountOption_PasswordExpired, &edits, this);

    new UnlockEdit(&edits, this);

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

    // Turn on "password expired" by default
    pass_expired_check->set_checked(true);

    connect(
        ok_button, &QPushButton::clicked,
        this, &PasswordDialog::accept);
    connect(
        cancel_button, &QPushButton::clicked,
        this, &QDialog::reject);
}

void PasswordDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    show_busy_indicator();

    const bool verify_success = edits_verify(ad, edits, target);
    if (!verify_success) {
        return;
    }

    const bool apply_success = edits_apply(ad, edits, target);

    hide_busy_indicator();

    STATUS()->display_ad_messages(ad, this);

    if (apply_success) {
        QDialog::accept();
    }
}
