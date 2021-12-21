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
#include "attribute_edits/attribute_edit.h"
#include "globals.h"
#include "status.h"
#include "utils.h"

#include <QLineEdit>

void RenameObjectDialog::success_msg(const QString &old_name) {
    const QString message = QString(tr("Object %1 was renamed.")).arg(old_name);
    g_status->add_message(message, StatusType_Success);
}

void RenameObjectDialog::fail_msg(const QString &old_name) {
    const QString message = QString(tr("Failed to rename object %1")).arg(old_name);
    g_status->add_message(message, StatusType_Error);
}

void RenameObjectDialog::init(AdInterface &ad, const QString &target_arg, QLineEdit *name_edit_arg, const QList<AttributeEdit *> &edits_arg) {
    name_edit = name_edit_arg;
    edits = edits_arg;
    target = target_arg;

    const QString name = dn_get_name(target);
    name_edit->setText(name);

    const AdObject object = ad.search_object(target);
    AttributeEdit::load(edits, ad, object);
}

void RenameObjectDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    const QString old_dn = target;
    const QString old_name = dn_get_name(target);

    const bool verify_success = AttributeEdit::verify(ad, edits, target);
    if (!verify_success) {
        return;
    }

    show_busy_indicator();

    const QString new_name = get_new_name();
    const QString new_dn = get_new_dn();

    const bool rename_success = ad.object_rename(target, new_name);

    bool final_success = false;
    if (rename_success) {
        const bool apply_success = AttributeEdit::apply(ad, edits, new_dn);

        if (apply_success) {
            final_success = true;
        }
    }

    hide_busy_indicator();

    g_status->display_ad_messages(ad, this);

    if (final_success) {
        RenameObjectDialog::success_msg(old_name);

        QDialog::accept();
    } else {
        RenameObjectDialog::fail_msg(old_name);
    }
}

QString RenameObjectDialog::get_new_name() const {
    const QString new_name = name_edit->text().trimmed();

    return new_name;
}

QString RenameObjectDialog::get_new_dn() const {
    const QString new_name = get_new_name();
    const QString new_dn = dn_rename(target, new_name);

    return new_dn;
}
