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

#include "create_dialog.h"
#include "ad_interface.h"
#include "utils.h"
#include "status.h"
#include "edits/attribute_edit.h"
#include "edits/string_edit.h"
#include "edits/group_scope_edit.h"
#include "edits/group_type_edit.h"
#include "edits/account_option_edit.h"
#include "edits/password_edit.h"
#include "utils.h"
#include "settings.h"
#include "config.h"

#include <QDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QList>
#include <QComboBox>
#include <QMessageBox>
#include <QCheckBox>
#include <QPushButton>

// TODO: implement checkbox for account option "User cannot change password". Can't just do it through UAC attribute bits.

QString create_type_to_string(const CreateType &type);

CreateDialog::CreateDialog(const QString &parent_dn_arg, CreateType type_arg)
: QDialog()
{
    parent_dn = parent_dn_arg;
    type = type_arg;

    setAttribute(Qt::WA_DeleteOnClose);

    const QString type_string = create_type_to_string(type);
    const QString parent_canonical = dn_canonical(parent_dn);
    const auto title = QString(tr("Create %1 in \"%2\" - %3")).arg(type_string, parent_canonical, ADMC_APPLICATION_NAME);
    setWindowTitle(title);
    
    const QString object_class =
    [this]() {
        switch (type) {
            case CreateType_User: return CLASS_USER;
            case CreateType_Computer: return CLASS_COMPUTER;
            case CreateType_OU: return CLASS_OU;
            case CreateType_Group: return CLASS_GROUP;
            case CreateType_COUNT: return "";
        }
        return "";
    }();

    name_edit = new QLineEdit();

    const auto edits_layout = new QFormLayout();

    pass_edit = nullptr;

    switch (type) {
        case CreateType_User: {
            auto first_name_edit = new StringEdit(ATTRIBUTE_FIRST_NAME, object_class, this, &all_edits);
            auto last_name_edit = new StringEdit(ATTRIBUTE_LAST_NAME, object_class, this, &all_edits);
            auto initials_edit = new StringEdit(ATTRIBUTE_INITIALS, object_class, this, &all_edits);
            auto upn_edit = new StringEdit(ATTRIBUTE_USER_PRINCIPAL_NAME, object_class, this, &all_edits);
            auto sama_edit = new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, object_class, this, &all_edits);

            pass_edit = new PasswordEdit(this, &all_edits);

            const QList<AccountOption> options = {
                AccountOption_PasswordExpired,
                AccountOption_DontExpirePassword,
                AccountOption_Disabled
                // TODO: AccountOption_CannotChangePass
            };
            QMap<AccountOption, AccountOptionEdit *> option_edits;
            AccountOptionEdit::make(options, &option_edits, &all_edits, this);

            // NOTE: initials not required
            required_edits = {
                first_name_edit,
                last_name_edit,
                upn_edit,
                sama_edit,
                pass_edit
            };

            first_name_edit->add_to_layout(edits_layout);
            last_name_edit->add_to_layout(edits_layout);
            edits_layout->addRow(tr("Full name:"), name_edit);
            initials_edit->add_to_layout(edits_layout);
            upn_edit->add_to_layout(edits_layout);
            sama_edit->add_to_layout(edits_layout);
            pass_edit->add_to_layout(edits_layout);
            option_edits[AccountOption_PasswordExpired]->add_to_layout(edits_layout);
            option_edits[AccountOption_DontExpirePassword]->add_to_layout(edits_layout);
            option_edits[AccountOption_Disabled]->add_to_layout(edits_layout);

            // Setup autofills
            // (first name + last name) -> full name
            auto autofill_full_name =
            [=]() {
                const QString full_name_value =
                [=]() {
                    const QString first_name = first_name_edit->get_input(); 
                    const QString last_name = last_name_edit->get_input(); 

                    const bool last_name_first = SETTINGS()->get_bool(BoolSetting_LastNameBeforeFirstName);
                    if (last_name_first) {
                        return last_name + " " + first_name;
                    } else {
                        return first_name + " " + last_name;
                    }
                }();

                // TODO: replace with full name
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
                upn_edit, &StringEdit::edited,
                [=] () {
                    const QString upn_input = upn_edit->get_input();
                    sama_edit->set_input(upn_input);
                });

            break;
        }
        case CreateType_Group: {
            auto sama_edit = new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, object_class, this, &all_edits);

            required_edits = {
                sama_edit
            };

            new GroupScopeEdit(this, &all_edits);
            new GroupTypeEdit(this, &all_edits);

            edits_layout->addRow(tr("Name:"), name_edit);
            edits_add_to_layout(all_edits, edits_layout);

            break;
        }
        case CreateType_Computer: {
            // TODO: "Assign this computer account as a pre-Windows 2000 computer". Is this needed?

            // TODO: "The following user or group may join this computer to a domain". Tried to figure out how this is implemented and couldn't see any easy ways via attributes, so probably something to do with setting ACL'S.

            // TODO: "This is a managed computer" checkbox and an edit for guid/uuid which I assume modifies objectGUID?

            auto sama_edit = new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, object_class, this, &all_edits);

            required_edits = {
                sama_edit
            };

            edits_layout->addRow(tr("Name:"), name_edit);
            sama_edit->add_to_layout(edits_layout);

            // Autofill name -> sama
            QObject::connect(
                name_edit, &QLineEdit::textChanged,
                [=] () {
                    const QString name_input = name_edit->text();
                    sama_edit->set_input(name_input.toUpper());
                });

            break;
        }
        case CreateType_OU: {
            edits_layout->addRow(tr("Name:"), name_edit);

            break;
        }
        case CreateType_COUNT: {
            break;
        }
    }

    create_button = new QPushButton(tr("Create"));

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(edits_layout);
    layout->addWidget(create_button);

    connect(
        create_button, &QAbstractButton::clicked,
        this, &CreateDialog::accept);

    for (auto edit : required_edits) {
        connect(
            edit, &AttributeEdit::edited,
            this, &CreateDialog::on_edited);
    }
    connect(
        name_edit, &QLineEdit::textChanged,
        this, &CreateDialog::on_edited);
    on_edited();
}

void CreateDialog::accept() {
    const bool pass_confirmed = pass_edit->check_confirm();
    if (!pass_confirmed) {
        return;
    }
    
    const QString name = name_edit->text();

    const QString suffix =
    [this]() {
        switch (type) {
            case CreateType_User: return "CN";
            case CreateType_Computer: return "CN";
            case CreateType_OU: return "OU";
            case CreateType_Group: return "CN";
            case CreateType_COUNT: return "COUNT";
        }
        return "";
    }();

    const QString dn = suffix + "=" + name + "," + parent_dn;

    const char **classes =
    [this]() {
        static const char *classes_user[] = {CLASS_USER, NULL};
        static const char *classes_group[] = {CLASS_GROUP, NULL};
        static const char *classes_ou[] = {CLASS_OU, NULL};
        static const char *classes_computer[] = {CLASS_TOP, CLASS_PERSON, CLASS_ORG_PERSON, CLASS_USER, CLASS_COMPUTER, NULL};

        switch (type) {
            case CreateType_User: return classes_user;
            case CreateType_Computer: return classes_computer;
            case CreateType_OU: return classes_ou;
            case CreateType_Group: return classes_group;
            case CreateType_COUNT: return classes_user;
        }
        return classes_user;
    }();

    const int errors_index = Status::instance()->get_errors_size();
    AD()->start_batch();
    {   
        const QString type_string = create_type_to_string(type);

        auto fail_msg =
        [this, type_string, name]() {
            const QString message = QString(tr("Failed to create %1 - \"%2\"")).arg(type_string, name);
            Status::instance()->message(message, StatusType_Error);
        };

        const bool add_success = AD()->object_add(dn, classes);

        if (add_success) {
            const bool apply_if_unmodified = true;
            const bool apply_success = edits_apply(all_edits, dn, apply_if_unmodified);

            if (apply_success) {
                const QString message = QString(tr("Created %1 - \"%2\"")).arg(type_string, name);

                Status::instance()->message(message, StatusType_Success);

                QDialog::accept();
            } else {
                AD()->object_delete(dn);
                fail_msg();
            }
        } else {
            fail_msg();
        }
    }
    AD()->end_batch();
    Status::instance()->show_errors_popup(errors_index);
}

// Enable/disable create button if all required edits filled
void CreateDialog::on_edited() {
    const bool required_edits_filled =
    [this]() {
        for (auto edit : required_edits) {
            if (!edit->modified()) {
                return false;
            }
        }

        return true;
    }();

    const bool name_filled = !name_edit->text().isEmpty();

    create_button->setEnabled(required_edits_filled && name_filled);
}

QString create_type_to_string(const CreateType &type) {
    switch (type) {
        case CreateType_User: return CreateDialog::tr("User");
        case CreateType_Computer: return CreateDialog::tr("Computer");
        case CreateType_OU: return CreateDialog::tr("Organization Unit");
        case CreateType_Group: return CreateDialog::tr("Group");
        case CreateType_COUNT: return "COUNT";
    }
    return "";
}
