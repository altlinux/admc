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

#include "create_object_dialog.h"

#include "adldap.h"
#include "attribute_edits/attribute_edit.h"
#include "globals.h"
#include "status.h"
#include "utils.h"

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>

void CreateObjectDialog::init(QLineEdit *name_edit_arg, QDialogButtonBox *button_box, const QList<AttributeEdit *> &edits_list, const QList<QLineEdit *> &required_list, const QString &object_class) {
    name_edit = name_edit_arg;
    m_edit_list = edits_list;
    m_required_list = required_list;
    m_object_class = object_class;

    ok_button = button_box->button(QDialogButtonBox::Ok);

    for (QLineEdit *edit : m_required_list) {
        connect(
            edit, &QLineEdit::textChanged,
            this, &CreateObjectDialog::on_edited);
    }
    on_edited();
}

void CreateObjectDialog::set_parent_dn(const QString &dn) {
    parent_dn = dn;
}

void CreateObjectDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    const QString name = get_created_name();
    const QString dn = get_created_dn();

    // Verify edits
    const bool verify_success = edits_verify(ad, m_edit_list, dn);

    if (!verify_success) {
        return;
    }

    auto fail_msg = [name]() {
        const QString message = QString(tr("Failed to create object %1")).arg(name);
        g_status->add_message(message, StatusType_Error);
    };

    const bool add_success = ad.object_add(dn, m_object_class);

    bool final_success = false;
    if (add_success) {
        const bool apply_success = edits_apply(ad, m_edit_list, dn);

        if (apply_success) {
            final_success = true;
        } else {
            ad.object_delete(dn);
        }
    }

    g_status->display_ad_messages(ad, this);

    if (final_success) {
        const QString message = QString(tr("Object %1 was created")).arg(name);

        g_status->add_message(message, StatusType_Success);

        QDialog::accept();
    } else {
        fail_msg();
    }
}

// Enable/disable create button if all required edits filled
void CreateObjectDialog::on_edited() {
    const bool all_required_filled = [this]() {
        for (QLineEdit *edit : m_required_list) {
            if (edit->text().isEmpty()) {
                return false;
            }
        }

        return true;
    }();

    ok_button->setEnabled(all_required_filled);
}

QString CreateObjectDialog::get_created_name() const {
    // NOTE: trim whitespaces because server will do it
    // anyway and we want a correct name
    const QString name = name_edit->text().trimmed();

    return name;
}

QString CreateObjectDialog::get_created_dn() const {
    const QString name = get_created_name();
    const QString dn = dn_from_name_and_parent(name, parent_dn, m_object_class);

    return dn;
}
