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

#include "create_object_helper.h"

#include "adldap.h"
#include "attribute_edits/attribute_edit.h"
#include "globals.h"
#include "status.h"
#include "utils.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>

// TODO: the logic of "enable/disable ok button
// depending on whether all required edits contain
// input" is duplicated in password dialog and maybe in
// other places. Can create an abstraction for it to
// reduce duplication.

CreateObjectHelper::CreateObjectHelper(QLineEdit *name_edit_arg, QDialogButtonBox *button_box, const QList<AttributeEdit *> &edits_list, const QList<QLineEdit *> &required_list, const QString &object_class, const QString &parent_dn_arg, QDialog *parent_dialog_arg)
: QObject(parent_dialog_arg) {
    parent_dialog = parent_dialog_arg;
    name_edit = name_edit_arg;
    m_edit_list = edits_list;
    m_required_list = required_list;
    m_object_class = object_class;
    parent_dn = parent_dn_arg;

    ok_button = button_box->button(QDialogButtonBox::Ok);

    limit_edit(name_edit, ATTRIBUTE_CN);

    for (QLineEdit *edit : m_required_list) {
        connect(
            edit, &QLineEdit::textChanged,
            this, &CreateObjectHelper::on_edited);
    }
    on_edited();
}

bool CreateObjectHelper::accept() const {
    AdInterface ad;
    if (ad_failed(ad, parent_dialog)) {
        return false;
    }

    const QString name = get_created_name();
    const QString dn = get_created_dn();

    auto fail_msg = [name]() {
        const QString message = QString(tr("Failed to create object %1")).arg(name);
        g_status->add_message(message, StatusType_Error);
    };

    const bool verify_name_success = verify_object_name(name, parent_dialog);
    if (!verify_name_success) {
        fail_msg();
        return false;
    }

    // Verify edits
    const bool verify_success = AttributeEdit::verify(m_edit_list, ad, dn);

    if (!verify_success) {
        fail_msg();
        return false;
    }

    bool final_success = true;

    const QHash<QString, QList<QString>> attr_map = [&]() {
        if (m_object_class == CLASS_SHARED_FOLDER) {
            // NOTE: for shared folders, UNC name must
            // be defined on creation because it's a
            // mandatory attribute
            return QHash<QString, QList<QString>>({
                {ATTRIBUTE_OBJECT_CLASS, {m_object_class}},
                {ATTRIBUTE_UNC_NAME, {"placeholder"}},
            });
        } else {
            return QHash<QString, QList<QString>>({
                {ATTRIBUTE_OBJECT_CLASS, {m_object_class}},
            });
        }
    }();

    const bool add_success = ad.object_add(dn, attr_map);

    final_success = (final_success && add_success);

    if (add_success) {
        const bool is_user_or_person = (m_object_class == CLASS_USER || m_object_class == CLASS_INET_ORG_PERSON);
        const bool is_computer = (m_object_class == CLASS_COMPUTER);

        if (is_user_or_person) {
            const int uac = [dn, &ad]() {
                const AdObject object = ad.search_object(dn, {ATTRIBUTE_USER_ACCOUNT_CONTROL});
                const int out = object.get_int(ATTRIBUTE_USER_ACCOUNT_CONTROL);

                return out;
            }();

            const int bit = UAC_PASSWD_NOTREQD;
            const int updated_uac = bitmask_set(uac, bit, false);

            final_success = (final_success && ad.attribute_replace_int(dn, ATTRIBUTE_USER_ACCOUNT_CONTROL, updated_uac, DoStatusMsg_No));
        } else if (is_computer) {
            // NOTE: other attributes like primary
            // group and sam account type are
            // automatically changed by the server when
            // we set UAC to the correct value
            const int uac = (UAC_PASSWD_NOTREQD | UAC_WORKSTATION_TRUST_ACCOUNT);
            final_success = (final_success && ad.attribute_replace_int(dn, ATTRIBUTE_USER_ACCOUNT_CONTROL, uac));
        }

        const bool apply_success = AttributeEdit::apply(m_edit_list, ad, dn);

        final_success = (final_success && apply_success);
    }

    if (!final_success && add_success) {
        ad.object_delete(dn);
    }

    g_status->display_ad_messages(ad, parent_dialog);

    if (final_success) {
        const QString message = QString(tr("Object %1 was created")).arg(name);

        g_status->add_message(message, StatusType_Success);
    } else {
        fail_msg();
    }

    return final_success;
}

// Enable/disable create button if all required edits filled
void CreateObjectHelper::on_edited() {
    const bool all_required_filled = [this]() {
        QRegExp reg_exp_spaces("^\\s*$");
        for (QLineEdit *edit : m_required_list) {
            if (edit->text().isEmpty() || edit->text().contains(reg_exp_spaces)) {
                return false;
            }
        }

        return true;
    }();

    ok_button->setEnabled(all_required_filled);
}

QString CreateObjectHelper::get_created_name() const {
    // NOTE: trim whitespaces because server will do it
    // anyway and we want a correct name
    const QString name = name_edit->text().trimmed();

    return name;
}

QString CreateObjectHelper::get_created_dn() const {
    const QString name = get_created_name();
    const QString dn = dn_from_name_and_parent(name, parent_dn, m_object_class);

    return dn;
}
