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

#include "adldap.h"
#include "console_types/console_policy.h"
#include "globals.h"
#include "rename_object_dialog.h"
#include "status.h"
#include "utils.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QModelIndex>
#include <QPushButton>
#include <QVBoxLayout>

RenamePolicyDialog::RenamePolicyDialog(ConsoleWidget *console_arg)
: QDialog(console_arg) {
    console = console_arg;

    const auto title = QString(tr("Rename policy"));
    setWindowTitle(title);

    name_edit = new QLineEdit();

    auto button_box = new QDialogButtonBox();
    ok_button = button_box->addButton(QDialogButtonBox::Ok);
    reset_button = button_box->addButton(QDialogButtonBox::Reset);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);
    connect(
        ok_button, &QPushButton::clicked,
        this, &RenamePolicyDialog::accept);
    connect(
        reset_button, &QPushButton::clicked,
        this, &RenamePolicyDialog::reset);
    connect(
        cancel_button, &QPushButton::clicked,
        this, &RenamePolicyDialog::reject);

    const auto edits_layout = new QFormLayout();
    // NOTE: label name edit as "Name" even though it edits
    // display name attribute
    edits_layout->addRow(tr("Name:"), name_edit);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addLayout(edits_layout);
    top_layout->addWidget(button_box);

    connect(
        name_edit, &QLineEdit::textChanged,
        this, &RenamePolicyDialog::on_edited);
    on_edited();
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

    const QModelIndex scope_index = get_selected_scope_index(console);
    const QString old_name = scope_index.data(Qt::DisplayRole).toString();

    const QString new_name = name_edit->text();
    const bool apply_success = ad.attribute_replace_string(target, ATTRIBUTE_DISPLAY_NAME, new_name);

    if (apply_success) {
        RenameObjectDialog::success_msg(old_name);
        QDialog::accept();
    } else {
        RenameObjectDialog::fail_msg(old_name);
    }

    g_status()->display_ad_messages(ad, this);

    const QString dn = scope_index.data(PolicyRole_DN).toString();
    const AdObject object = ad.search_object(dn);

    QStandardItem *scope_item = console->get_scope_item(scope_index);
    console_policy_scope_load(scope_item, object);

    const QModelIndex results_index = console_item_get_buddy(scope_index);
    const QList<QStandardItem *> results_row = console->get_results_row(results_index);
    console_policy_results_load(results_row, object);

    console->sort_scope();
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
        const QModelIndex scope_index = get_selected_scope_index(console);
        const QString dn = scope_index.data(PolicyRole_DN).toString();
        const AdObject object = ad.search_object(dn);

        return object.get_string(ATTRIBUTE_DISPLAY_NAME);
    }();
    name_edit->setText(name);

    reset_button->setEnabled(false);
    ok_button->setEnabled(false);
}
