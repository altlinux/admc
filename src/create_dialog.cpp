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
#include "attribute_widgets.h"

#include <QDialog>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QList>
#include <QComboBox>
#include <QMessageBox>
#include <QCheckBox>

// TODO: implement cannot change pass

void autofill_edit_text_from_other_edit(QLineEdit *from, QLineEdit *to);
QString create_type_to_string(const CreateType &type);

void create_dialog(const QString &parent_dn, CreateType type, QWidget *parent) {
    auto get_create_dialog =
    [=]() -> QDialog * {
        switch (type) {
            case CreateType_Group: return new CreateGroupDialog(parent_dn, CreateType_Group, parent);
            case CreateType_User: return new CreateUserDialog(parent_dn, CreateType_User, parent);
            default: return new CreateDialog(parent_dn, type, parent);
        }
        return nullptr;
    };

    const auto create_dialog = get_create_dialog();
    create_dialog->open();
}

CreateDialog::CreateDialog(const QString &parent_dn_arg, CreateType type_arg, QWidget *parent)
: QDialog(parent)
{
    parent_dn = parent_dn_arg;
    type = type_arg;

    setAttribute(Qt::WA_DeleteOnClose);
    resize(600, 600);

    layout = new QGridLayout();
    setLayout(layout);

    const QString type_string = create_type_to_string(type);
    const auto title_text = QString(CreateDialog::tr("Create %1 in \"%2\"")).arg(type_string, parent_dn);
    auto label = new QLabel(title_text);

    layout->addWidget(label);

    name_edit = new QLineEdit(this);
    layout_labeled_widget(layout, tr("Name"), name_edit);
}

void CreateDialog::on_ok() {
    const QString name = name_edit->text();

    auto get_suffix =
    [](CreateType type_arg) {
        switch (type_arg) {
            case CreateType_User: return "CN";
            case CreateType_Computer: return "CN";
            case CreateType_OU: return "OU";
            case CreateType_Group: return "CN";
            case CreateType_COUNT: return "COUNT";
        }
        return "";
    };
    const QString suffix = get_suffix(type);

    const QString dn = suffix + "=" + name + "," + parent_dn;

    auto get_classes =
    [](CreateType type_arg) {
        static const char *classes_user[] = {CLASS_USER, NULL};
        static const char *classes_group[] = {CLASS_GROUP, NULL};
        static const char *classes_ou[] = {CLASS_OU, NULL};
        static const char *classes_computer[] = {CLASS_TOP, CLASS_PERSON, CLASS_ORG_PERSON, CLASS_USER, CLASS_COMPUTER, NULL};

        switch (type_arg) {
            case CreateType_User: return classes_user;
            case CreateType_Computer: return classes_computer;
            case CreateType_OU: return classes_ou;
            case CreateType_Group: return classes_group;
            case CreateType_COUNT: return classes_user;
        }
        return classes_user;
    };
    const char **classes = get_classes(type);

    const AdResult result_add = AdInterface::instance()->object_add(dn, classes);
    QList<AdResult> results = { result_add };

    if (result_add.success) {
        const QList<AdResult> results_more = apply_more_widgets(dn);
        results.append(results_more);
    }

    const QString type_string = create_type_to_string(type);

    if (no_errors(results)) {
        const QString message = QString(CreateDialog::tr("Created %1 - \"%2\"")).arg(type_string, name);

        Status::instance()->message(message, StatusType_Success);
    } else {
        if (result_add.success) {
            AdInterface::instance()->object_delete(dn);
        }

        show_warnings_for_error_results(results, this);

        const QString message = QString(CreateDialog::tr("Failed to create %1 - \"%2\"")).arg(type_string, name);

        Status::instance()->message(message, StatusType_Error);
    }
}

void CreateDialog::add_ok_cancel_buttons() {
    const auto ok_button = new QPushButton(tr("OK"), this);
    const auto cancel_button = new QPushButton(tr("Cancel"), this);

    const int button_row = layout->rowCount();
    layout->addWidget(cancel_button, button_row, 0, Qt::AlignLeft);
    layout->addWidget(ok_button, button_row, 1, Qt::AlignRight);

    connect(
        ok_button, &QAbstractButton::clicked,
        this, &CreateDialog::on_ok);
    connect(
        cancel_button, &QAbstractButton::clicked,
        this, &QDialog::reject);
}

QList<AdResult> CreateDialog::apply_more_widgets(const QString &dn) {
    return QList<AdResult>();
}

CreateGroupDialog::CreateGroupDialog(const QString &parent_dn_arg, CreateType type_arg, QWidget *parent)
:CreateDialog(parent_dn_arg, type_arg, parent) {
    const QList<QString> attributes_to_make = {
        ATTRIBUTE_SAMACCOUNT_NAME,
    };
    make_attribute_edits(attributes_to_make, layout, &attributes);

    autofill_edit_text_from_other_edit(name_edit, attributes[ATTRIBUTE_SAMACCOUNT_NAME]);

    scope_combo = new QComboBox(this);
    for (int i = 0; i < GroupScope_COUNT; i++) {
        const GroupScope scope = (GroupScope) i;
        const QString scope_string = group_scope_to_string(scope);

        scope_combo->addItem(scope_string, (int)scope);
    }
    layout_labeled_widget(layout, tr("Group scope:"), scope_combo);

    type_combo = new QComboBox(this);
    for (int i = 0; i < GroupType_COUNT; i++) {
        const GroupType group_type = (GroupType) i;
        const QString type_string = group_type_to_string(group_type);

        type_combo->addItem(type_string, (int)group_type);
    }
    layout_labeled_widget(layout, tr("Group type:"), type_combo);

    add_ok_cancel_buttons();
}

QList<AdResult> CreateGroupDialog::apply_more_widgets(const QString &dn) {
    QList<AdResult> results;

    const GroupScope group_scope = (GroupScope)scope_combo->currentData().toInt();
    const GroupType group_type = (GroupType)scope_combo->currentData().toInt();

    results.append(apply_attribute_edits(attributes, dn));

    results.append(AdInterface::instance()->group_set_type(dn, group_type));
    results.append(AdInterface::instance()->group_set_scope(dn, group_scope));

    return results;
}

CreateUserDialog::CreateUserDialog(const QString &parent_dn_arg, CreateType type_arg, QWidget *parent)
:CreateDialog(parent_dn_arg, type_arg, parent) {
    // TODO: do password, make it share code with password dialog
    // make_edit(tr("Password"), &pass_edit);
    // make_edit(tr("Confirm password"), &pass_confirm_edit);

    const QList<QString> attributes_to_make = {
        ATTRIBUTE_FIRST_NAME,
        ATTRIBUTE_LAST_NAME,
        ATTRIBUTE_DISPLAY_NAME,
        ATTRIBUTE_INITIALS,
        ATTRIBUTE_USER_PRINCIPAL_NAME,
        ATTRIBUTE_SAMACCOUNT_NAME,
    };
    make_attribute_edits(attributes_to_make, layout, &attributes);

    autofill_edit_text_from_other_edit(name_edit, attributes[ATTRIBUTE_SAMACCOUNT_NAME]);

    const QList<AccountOption> options_to_make = {
        AccountOption_PasswordExpired,
        AccountOption_DontExpirePassword,
        AccountOption_Disabled
        // TODO:
        // AccountOption_CannotChangePass
    };
    make_account_option_checks(options_to_make, layout, &account_options);

    // When PasswordExpired is set, you can't set CannotChange and DontExpirePassword
    // Prevent the conflicting checks from being set when PasswordExpired is set already and show a message about it
    auto connect_never_expire_conflict =
    [this](AccountOption option) {
        QCheckBox *conflict = account_options[option]; 
        
        connect(conflict, &QCheckBox::stateChanged,
            [this, conflict, option]() {
                QCheckBox *pass_expired = account_options[AccountOption_PasswordExpired]; 
                
                if (checkbox_is_checked(pass_expired) && checkbox_is_checked(conflict)) {
                    conflict->setCheckState(Qt::Checked);

                    const QString pass_expired_text = get_account_option_description(AccountOption_PasswordExpired);
                    const QString conflict_text = get_account_option_description(option);
                    const QString error = QString(tr("Can't set \"%1\" when \"%2\" is set already.")).arg(conflict_text, pass_expired_text);

                    QMessageBox::warning(this, "Error", error);
                }
            }
            );
    };

    connect_never_expire_conflict(AccountOption_PasswordExpired);
    // connect_never_expire_conflict(AccountOption_CannotChange);

    // TODO: make full name auto-generate from first/last

    add_ok_cancel_buttons();
}

QList<AdResult> CreateUserDialog::apply_more_widgets(const QString &dn) {
    const QList<AdResult> results = apply_attribute_edits(attributes, dn);
    // TODO: account options and batching (for all other create dialogs too)

    return results;
}

// When "from" edit is edited, the text is copied to "to" edit
// But "to" can still be edited separately if needed
void autofill_edit_text_from_other_edit(QLineEdit *from, QLineEdit *to) {
    QObject::connect(
        from, &QLineEdit::textChanged,
        [=] () {
            to->setText(from->text());
        });
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
