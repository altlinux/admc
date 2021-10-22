/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "rename_policy_dialog.h"
#include "ui_rename_policy_dialog.h"

#include "adldap.h"
#include "globals.h"
#include "rename_dialog.h"
#include "status.h"
#include "utils.h"
#include "console_impls/policy_impl.h"
#include "console_impls/item_type.h"

#include <QPushButton>
#include <QModelIndex>

RenamePolicyDialog::RenamePolicyDialog(ConsoleWidget *console_arg)
: QDialog(console_arg) {
    ui = new Ui::RenamePolicyDialog();
    ui->setupUi(this);

    console = console_arg;

    ok_button = ui->button_box->button(QDialogButtonBox::Ok);
    reset_button = ui->button_box->button(QDialogButtonBox::Reset);

    connect(
        reset_button, &QPushButton::clicked,
        this, &RenamePolicyDialog::reset);
    connect(
        ui->name_edit, &QLineEdit::textChanged,
        this, &RenamePolicyDialog::on_edited);
    on_edited();
}

RenamePolicyDialog::~RenamePolicyDialog() {
    delete ui;
}

void RenamePolicyDialog::open() {
    reset();

    QDialog::open();
}

void RenamePolicyDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QModelIndex index = console->get_selected_item(ItemType_Policy);
    const QString old_name = index.data(Qt::DisplayRole).toString();

    const QString new_name = ui->name_edit->text();
    const bool apply_success = ad.attribute_replace_string(target, ATTRIBUTE_DISPLAY_NAME, new_name);

    if (apply_success) {
        RenameDialog::success_msg(old_name);
        QDialog::accept();
    } else {
        RenameDialog::fail_msg(old_name);
    }

    g_status()->display_ad_messages(ad, this);

    const QString dn = index.data(PolicyRole_DN).toString();
    const AdObject object = ad.search_object(dn);

    const QList<QStandardItem *> row = console->get_row(index);
    console_policy_load(row, object);
}

void RenamePolicyDialog::on_edited() {
    reset_button->setEnabled(true);
    ok_button->setEnabled(true);
}

void RenamePolicyDialog::reset() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QString name = [&]() {
        const QModelIndex index = console->get_selected_item(ItemType_Policy);
        const QString dn = index.data(PolicyRole_DN).toString();
        const AdObject object = ad.search_object(dn);

        return object.get_string(ATTRIBUTE_DISPLAY_NAME);
    }();
    ui->name_edit->setText(name);

    reset_button->setEnabled(false);
    ok_button->setEnabled(false);
}
