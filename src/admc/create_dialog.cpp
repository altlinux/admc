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

#include "create_dialog.h"

#include "adldap.h"
#include "edits/attribute_edit.h"
#include "globals.h"
#include "status.h"
#include "utils.h"

#include <QPushButton>
#include <QDialogButtonBox>
#include <QLineEdit>

void CreateDialog::init(QLineEdit *name_edit_arg, QDialogButtonBox *button_box, const QList<AttributeEdit *> &edits_list, const QList<QLineEdit *> &required_list, const QList<QWidget *> &widget_list, const QString &object_class) {
    name_edit = name_edit_arg;
    m_edit_list = edits_list;
    m_required_list = required_list;
    m_object_class = object_class;

    ok_button = button_box->button(QDialogButtonBox::Ok);

    // Save state once on initial creation, this will be
    // used as the default state that will be loaded to
    // reset the dialog
    m_state.set_widget_list(widget_list);
    m_state.save();

    for (QLineEdit *edit : m_required_list) {
        connect(
            edit, &QLineEdit::textChanged,
            this, &CreateDialog::on_edited);
    }
    on_edited();
}

QString CreateDialog::get_created_dn() const {
    const QString name = name_edit->text();
    const QString dn = dn_from_name_and_parent(name, parent_dn, m_object_class);

    return dn;
}

void CreateDialog::set_parent_dn(const QString &dn) {
    parent_dn = dn;
}

void CreateDialog::open() {
    m_state.restore();

    QDialog::open();
}

void CreateDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QString name = name_edit->text();

    const QString dn = get_created_dn();

    // Verify edits
    const bool verify_success = edits_verify(ad, m_edit_list, dn, true);

    if (!verify_success) {
        return;
    }

    auto fail_msg = [name]() {
        const QString message = QString(tr("Failed to create object %1")).arg(name);
        g_status()->add_message(message, StatusType_Error);
    };

    const bool add_success = ad.object_add(dn, m_object_class);

    bool final_success = false;
    if (add_success) {
        const bool apply_success = edits_apply(ad, m_edit_list, dn, true);

        if (apply_success) {
            final_success = true;

            QDialog::accept();
        } else {
            ad.object_delete(dn);
        }
    }

    g_status()->display_ad_messages(ad, this);

    if (final_success) {
        const QString message = QString(tr("Object %1 was created")).arg(name);

        g_status()->add_message(message, StatusType_Success);
    } else {
        fail_msg();
    }
}

// Enable/disable create button if all required edits filled
void CreateDialog::on_edited() {
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
