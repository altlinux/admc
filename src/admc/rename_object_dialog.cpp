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
#include "ui_rename_object_dialog.h"

#include "adldap.h"
#include "edits/string_edit.h"
#include "edits/upn_edit.h"
#include "globals.h"
#include "status.h"
#include "utils.h"

#include <QPushButton>

RenameObjectDialog::RenameObjectDialog(const QString &target_arg, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::RenameObjectDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    target = target_arg;

    AdInterface ad;
    if (ad_failed(ad)) {
        close();
        return;
    }

    const AdObject object = ad.search_object(target);

    const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);

    const QString class_name = g_adconfig->get_class_display_name(object_class);
    const auto title = tr("Rename Object - %1").arg(class_name);;
    setWindowTitle(title);

    if (object.is_class(CLASS_USER)) {
        new StringEdit(ATTRIBUTE_FIRST_NAME, object_class, &all_edits, this);
        new StringEdit(ATTRIBUTE_LAST_NAME, object_class, &all_edits, this);
        new StringEdit(ATTRIBUTE_DISPLAY_NAME, object_class, &all_edits, this);
        new UpnEdit(&all_edits, ad, this);
        new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, object_class, &all_edits, this);
    } else if (object.is_class(CLASS_GROUP)) {
        auto sama_edit = new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, object_class, &all_edits, this);
        sama_edit->get_edit()->setObjectName("sama_edit");
    }

    ok_button = ui->button_box->button(QDialogButtonBox::Ok);
    reset_button = ui->button_box->button(QDialogButtonBox::Reset);

    const QString name_edit_label = [&]() {
        if (object.is_class(CLASS_USER)) {
            return tr("Name:", "In Russian this needs to be different from just <Name> because in Russian <First Name> translates to <Name> as well and there's a First name edit below this one.");
        } else {
            return tr("Name:");
        }
    }();

    edits_add_to_layout(all_edits, ui->form_layout);

    connect(
        reset_button, &QPushButton::clicked,
        this, &RenameObjectDialog::reset);
    for (auto edit : all_edits) {
        connect(
            edit, &AttributeEdit::edited,
            this, &RenameObjectDialog::on_edited);
    }
    connect(
        ui->name_edit, &QLineEdit::textChanged,
        this, &RenameObjectDialog::on_edited);
    on_edited();

    reset();

    g_status()->display_ad_messages(ad, this);
}

void RenameObjectDialog::success_msg(const QString &old_name) {
    const QString message = QString(tr("Object %1 was renamed.")).arg(old_name);
    g_status()->add_message(message, StatusType_Success);
}

void RenameObjectDialog::fail_msg(const QString &old_name) {
    const QString message = QString(tr("Failed to rename object %1")).arg(old_name);
    g_status()->add_message(message, StatusType_Error);
}

QString RenameObjectDialog::get_new_dn() const {
    const QString new_name = ui->name_edit->text();
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

    const QString new_name = ui->name_edit->text();
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
    ui->name_edit->setText(name);

    const AdObject object = ad.search_object(target);
    edits_load(all_edits, ad, object);

    reset_button->setEnabled(false);
    ok_button->setEnabled(false);
}
