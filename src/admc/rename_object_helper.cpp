/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "rename_object_helper.h"

#include "adldap.h"
#include "attribute_edits/attribute_edit.h"
#include "globals.h"
#include "status.h"
#include "utils.h"

#include <QDialog>
#include <QLineEdit>

void RenameObjectHelper::success_msg(const QString &old_name) {
    const QString message = QString(tr("Object %1 was renamed.")).arg(old_name);
    g_status->add_message(message, StatusType_Success);
}

void RenameObjectHelper::fail_msg(const QString &old_name) {
    const QString message = QString(tr("Failed to rename object %1")).arg(old_name);
    g_status->add_message(message, StatusType_Error);
}

RenameObjectHelper::RenameObjectHelper(AdInterface &ad, const QString &target_arg, QLineEdit *name_edit_arg, const QList<AttributeEdit *> &edits_arg, QDialog *parent_dialog_arg, QList<QLineEdit *> required, QDialogButtonBox *button_box)
: QObject(parent_dialog_arg) {
    name_edit = name_edit_arg;
    edits = edits_arg;
    target = target_arg;
    parent_dialog = parent_dialog_arg;
    required_list = required;
    ok_button = nullptr;
    if (button_box != nullptr) {
        ok_button = button_box->button(QDialogButtonBox::Ok);
    }

    const QString name = dn_get_name(target);
    name_edit->setText(name);

    limit_edit(name_edit, ATTRIBUTE_CN);

    const AdObject object = ad.search_object(target);
    AttributeEdit::load(edits, ad, object);

    if (!required_list.isEmpty() && ok_button != nullptr) {
        for (QLineEdit *edit : required_list) {
            connect(edit, &QLineEdit::textChanged, this, &RenameObjectHelper::on_edited);
        }
        on_edited();
    }
}

bool RenameObjectHelper::accept() const {
    AdInterface ad;
    if (ad_failed(ad, parent_dialog)) {
        return false;
    }

    const QString old_dn = target;
    const QString old_name = dn_get_name(target);

    const QString new_name = get_new_name();

    const bool verify_name_success = verify_object_name(new_name, parent_dialog);
    if (!verify_name_success) {
        return false;
    }

    const bool verify_success = AttributeEdit::verify(edits, ad, target);
    if (!verify_success) {
        return false;
    }

    show_busy_indicator();

    const QString new_dn = get_new_dn();

    const bool rename_success = ad.object_rename(target, new_name);

    bool final_success = false;
    if (rename_success) {
        const bool apply_success = AttributeEdit::apply(edits, ad, new_dn);

        if (apply_success) {
            final_success = true;
        }
    }

    hide_busy_indicator();

    g_status->display_ad_messages(ad, parent_dialog);

    if (final_success) {
        RenameObjectHelper::success_msg(old_name);
    } else {
        RenameObjectHelper::fail_msg(old_name);
    }

    return final_success;
}

QString RenameObjectHelper::get_new_name() const {
    const QString new_name = name_edit->text().trimmed();

    return new_name;
}

QString RenameObjectHelper::get_new_dn() const {
    const QString new_name = get_new_name();
    const QString new_dn = dn_rename(target, new_name);

    return new_dn;
}

void RenameObjectHelper::on_edited() {
    const bool all_required_filled = [this]() {
        QRegExp reg_exp_spaces("^\\s*$");
        for (QLineEdit *edit : required_list) {
            if (edit->text().isEmpty() || edit->text().contains(reg_exp_spaces)) {
                return false;
            }
        }

        return true;
    }();

    ok_button->setEnabled(all_required_filled);
}
