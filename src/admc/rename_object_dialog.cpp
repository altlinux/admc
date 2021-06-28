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

#include "rename_object_dialog.h"
#include "adldap.h"
#include "edits/string_edit.h"
#include "globals.h"
#include "status.h"
#include "utils.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

RenameObjectDialog::RenameObjectDialog(const QString &target_arg, QWidget *parent)
: QDialog(parent) {
    setAttribute(Qt::WA_DeleteOnClose);

    target = target_arg;

    AdInterface ad;
    if (ad_failed(ad)) {
        close();
        return;
    }

    const AdObject object = ad.search_object(target);

    const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);

    const QString type_string = g_adconfig->get_class_display_name(object_class);
    const auto title = QString(tr("Rename object \"%1\"")).arg(type_string);
    setWindowTitle(title);

    name_edit = new QLineEdit();
    name_edit->setObjectName("name_edit");

    if (object.is_class(CLASS_USER)) {
        const QList<QString> attributes = {
            ATTRIBUTE_FIRST_NAME,
            ATTRIBUTE_LAST_NAME,
            ATTRIBUTE_DISPLAY_NAME,
            ATTRIBUTE_USER_PRINCIPAL_NAME,
            ATTRIBUTE_SAMACCOUNT_NAME,
        };
        StringEdit::make_many(attributes, object_class, &all_edits, this);
    } else if (object.is_class(CLASS_GROUP)) {
        auto sama_edit = new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, object_class, &all_edits, this);
        sama_edit->get_edit()->setObjectName("sama_edit");
    }

    auto button_box = new QDialogButtonBox();
    ok_button = button_box->addButton(QDialogButtonBox::Ok);
    reset_button = button_box->addButton(QDialogButtonBox::Reset);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);
    connect(
        ok_button, &QPushButton::clicked,
        this, &QDialog::accept);
    connect(
        reset_button, &QPushButton::clicked,
        this, &RenameObjectDialog::reset);
    connect(
        cancel_button, &QPushButton::clicked,
        this, &RenameObjectDialog::reject);

    const auto edits_layout = new QFormLayout();
    edits_layout->addRow(tr("Name:"), name_edit);
    edits_add_to_layout(all_edits, edits_layout);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addLayout(edits_layout);
    top_layout->addWidget(button_box);

    for (auto edit : all_edits) {
        connect(
            edit, &AttributeEdit::edited,
            this, &RenameObjectDialog::on_edited);
    }
    connect(
        name_edit, &QLineEdit::textChanged,
        this, &RenameObjectDialog::on_edited);
    on_edited();

    reset();

    g_status()->display_ad_messages(ad, this);
}

void RenameObjectDialog::success_msg(const QString &old_name) {
    const QString message = QString(tr("Renamed object \"%1\"")).arg(old_name);
    g_status()->add_message(message, StatusType_Success);
}

void RenameObjectDialog::fail_msg(const QString &old_name) {
    const QString message = QString(tr("Failed to rename object \"%1\"")).arg(old_name);
    g_status()->add_message(message, StatusType_Error);
}

QString RenameObjectDialog::get_new_dn() const {
    const QString new_name = name_edit->text();
    const QString new_dn = dn_rename(target, new_name);

    return new_dn;
}

void RenameObjectDialog::accept() {
    // Handle failure
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QString old_name = dn_get_name(target);

    const bool verify_success = edits_verify(ad, all_edits, target);
    if (!verify_success) {
        return;
    }

    show_busy_indicator();

    const QString new_name = name_edit->text();
    const bool rename_success = ad.object_rename(target, new_name);

    bool final_success = false;
    if (rename_success) {
        const QString new_dn = dn_rename(target, new_name);
        const bool apply_success = edits_apply(ad, all_edits, new_dn);

        if (apply_success) {
            final_success = true;

            QDialog::accept();
        }
    }

    hide_busy_indicator();

    g_status()->display_ad_messages(ad, this);

    if (final_success) {
        success_msg(old_name);
    } else {
        fail_msg(old_name);
    }
}

void RenameObjectDialog::on_edited() {
    reset_button->setEnabled(true);
    ok_button->setEnabled(true);
}

void RenameObjectDialog::reset() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QString name = dn_get_name(target);
    name_edit->setText(name);

    const AdObject object = ad.search_object(target);
    edits_load(all_edits, ad, object);

    reset_button->setEnabled(false);
    ok_button->setEnabled(false);
}
