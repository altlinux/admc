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

#include "rename_policy_dialog.h"
#include "rename_dialog.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "ad_object.h"
#include "ad_utils.h"
#include "status.h"
#include "utils.h"

#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>

RenamePolicyDialog::RenamePolicyDialog(const QString &target_arg, QWidget *parent)
: QDialog(parent)
{
    target = target_arg;

    setAttribute(Qt::WA_DeleteOnClose);

    AdInterface ad;
    if (!ad_is_connected(ad)) {
        close();
    }

    // TODO: handle failure, dialog should close
    const AdObject object = ad.search_object(target);

    const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);

    const QString type_string = ADCONFIG()->get_class_display_name(object_class);
    const auto title = QString(tr("Rename object \"%1\"")).arg(type_string);
    setWindowTitle(title);

    name_edit = new QLineEdit();

    auto button_box = new QDialogButtonBox();
    ok_button = button_box->addButton(QDialogButtonBox::Ok);
    reset_button = button_box->addButton(QDialogButtonBox::Reset);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);
    connect(
        ok_button, &QPushButton::clicked,
        this, &QDialog::accept);
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

    reset();
}

void RenamePolicyDialog::accept() {
    AdInterface ad;
    if (ad_is_connected(ad)) {
        return;
    }

    const AdObject object = ad.search_object(target);
    const QString old_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);

    STATUS()->start_error_log();

    const QString new_name = name_edit->text();
    const bool apply_success = ad.attribute_replace_string(target, ATTRIBUTE_DISPLAY_NAME, new_name);

    if (apply_success) {
        RenameDialog::success_msg(old_name);
        QDialog::close();
    } else {
        RenameDialog::fail_msg(old_name);
    }

    STATUS()->end_error_log(this);
}

void RenamePolicyDialog::on_edited() {
    reset_button->setEnabled(true);
    ok_button->setEnabled(true);
}

void RenamePolicyDialog::reset() {
    AdInterface ad;
    if (!ad_is_connected(ad)) {
        return;
    }

    const AdObject object = ad.search_object(target);
    const QString name = object.get_string(ATTRIBUTE_DISPLAY_NAME);
    name_edit->setText(name);

    reset_button->setEnabled(false);
    ok_button->setEnabled(false);
}
