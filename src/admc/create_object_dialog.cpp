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
#include "edits/account_option_edit.h"
#include "edits/group_scope_edit.h"
#include "edits/group_type_edit.h"
#include "edits/password_edit.h"
#include "edits/string_edit.h"
#include "edits/upn_edit.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QButtonGroup>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

// TODO: implement checkbox for account option "User cannot change password". Can't just do it through UAC attribute bits.

// TODO: not sure about how required_edits are done, maybe
// just do this through verify()? Had to remove upnedit from
// required_edits because that's a list of stringedits. Now upnedit checks that it's not empty in verify();

CreateObjectDialog::CreateObjectDialog(const QString &parent_dn_arg, const QString &object_class_arg, QWidget *parent)
: QDialog(parent) {
    setAttribute(Qt::WA_DeleteOnClose);

    AdInterface ad;
    if (ad_failed(ad)) {
        close();

        return;
    }

    parent_dn = parent_dn_arg;
    object_class = object_class_arg;

    setMinimumWidth(400);

    const QString class_name = g_adconfig->get_class_display_name(object_class);
    const QString title = tr("Create Object - %1").arg(class_name);
    setWindowTitle(title);

    name_edit = new QLineEdit();
    name_edit->setObjectName("name_edit");

    const auto edits_layout = new QFormLayout();

    pass_edit = nullptr;

    if (object_class == CLASS_USER) {
        auto first_name_edit = new StringEdit(ATTRIBUTE_FIRST_NAME, object_class, &all_edits, this);
        auto last_name_edit = new StringEdit(ATTRIBUTE_LAST_NAME, object_class, &all_edits, this);
        auto initials_edit = new StringEdit(ATTRIBUTE_INITIALS, object_class, &all_edits, this);
        auto upn_edit = new UpnEdit(&all_edits, ad, this);
        auto sama_edit = new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, object_class, &all_edits, this);

        first_name_edit->get_edit()->setObjectName("first_name_edit");
        last_name_edit->get_edit()->setObjectName("last_name_edit");
        initials_edit->get_edit()->setObjectName("initials_edit");
        upn_edit->get_edit()->setObjectName("upn_edit");
        sama_edit->get_edit()->setObjectName("sama_edit");

        pass_edit = new PasswordEdit(&all_edits, this);

        const QList<AccountOption> options = {
            AccountOption_PasswordExpired,
            AccountOption_CantChangePassword,
            AccountOption_DontExpirePassword,
            AccountOption_Disabled,
        };
        QMap<AccountOption, AccountOptionEdit *> option_edits;
        AccountOptionEdit::make_many(options, &option_edits, &all_edits, this);
        QWidget *options_widget = AccountOptionEdit::layout_many(options, option_edits);

        required_edits = {
            first_name_edit,
            sama_edit,
        };

        first_name_edit->add_to_layout(edits_layout);
        last_name_edit->add_to_layout(edits_layout);
        edits_layout->addRow(tr("Full name:"), name_edit);
        initials_edit->add_to_layout(edits_layout);
        upn_edit->add_to_layout(edits_layout);
        sama_edit->add_to_layout(edits_layout);
        pass_edit->add_to_layout(edits_layout);
        edits_layout->addRow(options_widget);

        // Setup autofills
        // (first name + last name) -> full name
        auto autofill_full_name = [=]() {
            const QString full_name_value = [=]() {
                const QString first_name = first_name_edit->get_input();
                const QString last_name = last_name_edit->get_input();

                const bool last_name_first = settings_get_bool(SETTING_last_name_before_first_name);
                if (!first_name.isEmpty() && !last_name.isEmpty()) {
                    if (last_name_first) {
                        return last_name + " " + first_name;
                    } else {
                        return first_name + " " + last_name;
                    }
                } else if (!first_name.isEmpty()) {
                    return first_name;
                } else if (!last_name.isEmpty()) {
                    return last_name;
                } else {
                    return QString();
                }
            }();

            name_edit->setText(full_name_value);
        };
        QObject::connect(
            first_name_edit, &StringEdit::edited,
            autofill_full_name);
        QObject::connect(
            last_name_edit, &StringEdit::edited,
            autofill_full_name);

        // upn -> samaccount name
        QObject::connect(
            upn_edit, &UpnEdit::edited,
            [=]() {
                const QString upn_input = upn_edit->get_input();
                sama_edit->set_input(upn_input);
            });
    } else if (object_class == CLASS_GROUP) {
        auto sama_edit = new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, object_class, &all_edits, this);
        sama_edit->get_edit()->setObjectName("sama_edit");

        required_edits = {
            sama_edit,
        };

        new GroupScopeEdit(&all_edits, this);
        new GroupTypeEdit(&all_edits, this);

        edits_layout->addRow(tr("Name:"), name_edit);
        edits_add_to_layout(all_edits, edits_layout);
    } else if (object_class == CLASS_COMPUTER) {
        // TODO: "Assign this computer account as a pre-Windows 2000 computer". Is this needed?

        // TODO: "The following user or group may join this computer to a domain". Tried to figure out how this is implemented and couldn't see any easy ways via attributes, so probably something to do with setting ACL'S.

        // TODO: "This is a managed computer" checkbox and an edit for guid/uuid which I assume modifies objectGUID?

        auto sama_edit = new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, object_class, &all_edits, this);
        sama_edit->get_edit()->setObjectName("sama_edit");

        required_edits = {
            sama_edit};

        edits_layout->addRow(tr("Name:"), name_edit);
        sama_edit->add_to_layout(edits_layout);

        // Autofill name -> sama
        QObject::connect(
            name_edit, &QLineEdit::textChanged,
            [=]() {
                const QString name_input = name_edit->text();
                sama_edit->set_input(name_input.toUpper());
            });
    } else if (object_class == CLASS_OU) {
        edits_layout->addRow(tr("Name:"), name_edit);
    } else {
        qWarning() << "Class" << object_class << "is unsupported by create dialog";
        return;
    }

    auto button_box = new QDialogButtonBox();
    create_button = button_box->addButton(tr("Create"), QDialogButtonBox::AcceptRole);
    button_box->addButton(QDialogButtonBox::Cancel);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(edits_layout);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);

    for (auto edit : required_edits) {
        connect(
            edit, &AttributeEdit::edited,
            this, &CreateObjectDialog::on_edited);
    }
    connect(
        name_edit, &QLineEdit::textChanged,
        this, &CreateObjectDialog::on_edited);
    on_edited();
}

QString CreateObjectDialog::get_created_dn() const {
    const QString name = name_edit->text();
    const QString dn = dn_from_name_and_parent(name, parent_dn, object_class);

    return dn;
}

// NOTE: passing "ignore_modified" to verify and apply f-ns
// because this is a new object, so all the edits are in
// "unmodified" state but still need to be processed.
void CreateObjectDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QString name = name_edit->text();

    const QString dn = get_created_dn();

    // Verify edits
    const bool verify_success = edits_verify(ad, all_edits, dn, true);

    if (!verify_success) {
        return;
    }

    auto fail_msg = [name]() {
        const QString message = QString(tr("Failed to create object %1")).arg(name);
        g_status()->add_message(message, StatusType_Error);
    };

    const bool add_success = ad.object_add(dn, object_class);

    bool final_success = false;
    if (add_success) {
        // NOTE: pass "i
        const bool apply_success = edits_apply(ad, all_edits, dn, true);

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
void CreateObjectDialog::on_edited() {
    const bool required_edits_filled = [this]() {
        for (auto edit : required_edits) {
            if (edit->is_empty()) {
                return false;
            }
        }

        return true;
    }();

    const bool name_filled = !name_edit->text().isEmpty();

    create_button->setEnabled(required_edits_filled && name_filled);
}
