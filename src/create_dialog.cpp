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

const char **get_create_type_classes(const CreateType type);

void create_dialog(const QString &parent_dn, CreateType type, QWidget *parent) {
    auto get_create_dialog =
    [=]() -> QDialog * {
        switch (type) {
            case CreateType_Group: return new CreateGroupDialog(parent_dn, parent);
            case CreateType_User: return new CreateUserDialog(parent_dn, parent);
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

    const QString type_name = create_type_to_string(type);
    const auto title_label = new QLabel(QString(tr("Create object - \"%1\"")).arg(type_name), this);

    const auto ok_button = new QPushButton(tr("OK"), this);
    connect(
        ok_button, &QAbstractButton::clicked,
        this, &QDialog::accept);

    const auto cancel_button = new QPushButton(tr("Cancel"), this);
    connect(
        cancel_button, &QAbstractButton::clicked,
        this, &QDialog::reject);

    const auto label_layout = new QVBoxLayout();
    const auto widget_layout = new QVBoxLayout();

    const auto attributes_layout = new QHBoxLayout();
    attributes_layout->insertLayout(-1, label_layout);
    attributes_layout->insertLayout(-1, widget_layout);

    const auto top_layout = new QGridLayout(this);
    top_layout->addWidget(title_label, 0, 0);
    top_layout->addLayout(attributes_layout, 1, 0);
    top_layout->addWidget(cancel_button, 2, 0, Qt::AlignLeft);
    top_layout->addWidget(ok_button, 2, 2, Qt::AlignRight);

    {
        auto name_edit_label = new QLabel(tr("Name:"));
        name_edit = new QLineEdit(this);

        label_layout->addWidget(name_edit_label);
        widget_layout->addWidget(name_edit);
    }

    connect(
        this, &QDialog::accepted,
        this, &CreateDialog::on_accepted);
}

void CreateDialog::on_accepted() {
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
    
    const QString dn = suffix + "=" + name + "," + parent_dn;

    AdInterface::instance()->object_add(dn, classes);
}

QString create_type_to_string(const CreateType &type) {
    switch (type) {
        case CreateType_User: return AdInterface::tr("User");
        case CreateType_Computer: return AdInterface::tr("Computer");
        case CreateType_OU: return AdInterface::tr("Organization Unit");
        case CreateType_Group: return AdInterface::tr("Group");
        case CreateType_COUNT: return "COUNT";
    }
    return "";
}

CreateGroupDialog::CreateGroupDialog(const QString &parent_dn_arg, QWidget *parent)
: QDialog(parent)
{
    const CreateType type = CreateType_Group;

    parent_dn = parent_dn_arg;

    setAttribute(Qt::WA_DeleteOnClose);
    resize(600, 600);

    const QString type_name = create_type_to_string(type);
    const auto title_label = new QLabel(QString(tr("Create group \"%1\" in \"%2\"")).arg(type_name, parent_dn_arg), this);

    const auto ok_button = new QPushButton(tr("OK"), this);
    connect(
        ok_button, &QAbstractButton::clicked,
        this, &QDialog::accept);

    const auto cancel_button = new QPushButton(tr("Cancel"), this);
    connect(
        cancel_button, &QAbstractButton::clicked,
        this, &QDialog::reject);

    const auto label_layout = new QVBoxLayout();
    const auto widget_layout = new QVBoxLayout();

    const auto attributes_layout = new QHBoxLayout();
    attributes_layout->insertLayout(-1, label_layout);
    attributes_layout->insertLayout(-1, widget_layout);

    const auto top_layout = new QGridLayout(this);
    top_layout->addWidget(title_label, 0, 0);
    top_layout->addLayout(attributes_layout, 1, 0);
    top_layout->addWidget(cancel_button, 2, 0, Qt::AlignLeft);
    top_layout->addWidget(ok_button, 2, 2, Qt::AlignRight);

    auto add_to_layout =
    [label_layout, widget_layout](const QString label_text, QWidget *widget) {
        const auto label = new QLabel(label_text);

        label_layout->addWidget(label);
        widget_layout->addWidget(widget);
    };

    name_edit = new QLineEdit(this);
    add_to_layout(tr("Name:"), name_edit);

    sama_edit = new QLineEdit(this);
    add_to_layout(tr("Sama name:"), sama_edit);

    // Copy name into sama name when name changes
    connect(
        name_edit, &QLineEdit::textChanged,
        [this] () {
            sama_edit->setText(name_edit->text());
        });

    scope_combo = new QComboBox(this);
    for (int i = 0; i < GroupScope_COUNT; i++) {
        const GroupScope scope = (GroupScope) i;
        const QString scope_string = group_scope_to_string(scope);

        scope_combo->addItem(scope_string, (int)scope);
    }
    add_to_layout(tr("Group scope:"), scope_combo);

    type_combo = new QComboBox(this);
    for (int i = 0; i < GroupType_COUNT; i++) {
        const GroupType group_type = (GroupType) i;
        const QString type_string = group_type_to_string(group_type);

        type_combo->addItem(type_string, (int)group_type);
    }
    add_to_layout(tr("Group type:"), type_combo);

    connect(
        this, &QDialog::accepted,
        this, &CreateGroupDialog::on_accepted);
}

void CreateGroupDialog::on_accepted() {
    const QString name = name_edit->text();

    const QString sama_name = sama_edit->text();

    const GroupScope group_scope = (GroupScope)scope_combo->currentData().toInt();
    const GroupType group_type = (GroupType)scope_combo->currentData().toInt();

    const CreateType type = CreateType_Group;

    auto get_suffix =
    [type]() {
        switch (type) {
            case CreateType_User: return "CN";
            case CreateType_Computer: return "CN";
            case CreateType_OU: return "OU";
            case CreateType_Group: return "CN";
            case CreateType_COUNT: return "COUNT";
        }
        return "";
    };
    const QString suffix = get_suffix();

    const char **classes = get_create_type_classes(type);
    
    const QString dn = suffix + "=" + name + "," + parent_dn;

    // Create object and then apply attribute edits
    const AdResult result_add = AdInterface::instance()->object_add(dn, classes);
    QList<AdResult> results = {
        result_add
    };
    if (result_add.success) {
        const AdResult result_sama = AdInterface::instance()->attribute_replace(dn, ATTRIBUTE_SAMACCOUNT_NAME, sama_edit->text());
        const AdResult result_type = AdInterface::instance()->group_set_type(dn, group_type);
        const AdResult result_scope = AdInterface::instance()->group_set_scope(dn, group_scope);

        results.append({
            result_sama, result_type, result_scope
        });
    }

    // Determine if succeeded totally and open error popups for any
    // failed actions
    bool total_success = true;
    for (auto result : results) {
        if (!result.success) {
            total_success = false;

            QMessageBox::critical(this, "Error", result.error_with_context);
        }
    }

    if (total_success) {
        Status::instance()->message(QString(tr("Created group - \"%1\"")).arg(name), StatusType_Success);
    } else {
        // Delete object if it was added
        if (result_add.success) {
            AdInterface::instance()->object_delete(dn);
        }

        Status::instance()->message(QString(tr("Failed to create group - \"%1\"")).arg(name), StatusType_Error);
    }
}

CreateUserDialog::CreateUserDialog(const QString &parent_dn_arg, QWidget *parent)
: QDialog(parent)
{
    const CreateType type = CreateType_User;

    parent_dn = parent_dn_arg;

    setAttribute(Qt::WA_DeleteOnClose);

    const QString type_name = create_type_to_string(type);
    // TODO: maybe make general create type X in Y, switch on type. To reduce number of same strings for translation
    const auto title_label_text = QString(tr("Create \"%1\" in \"%2\"")).arg(type_name, parent_dn_arg);
    const auto title_label = new QLabel(title_label_text, this);

    const auto ok_button = new QPushButton(tr("OK"), this);
    connect(
        ok_button, &QAbstractButton::clicked,
        this, &QDialog::accept);

    const auto cancel_button = new QPushButton(tr("Cancel"), this);
    connect(
        cancel_button, &QAbstractButton::clicked,
        this, &QDialog::reject);

    const auto top_layout = new QGridLayout(this);
    top_layout->addWidget(title_label, 0, 0);

    auto add_to_layout =
    [top_layout](const QString label_text, QWidget *widget) {
        const auto label = new QLabel(label_text);

        const int row = top_layout->rowCount();

        top_layout->addWidget(label, row, 0);
        top_layout->addWidget(widget, row, 1);
    };

    auto make_check =
    [this, add_to_layout](AccountOption option, QCheckBox **check_ptr) {
        const QString label_text = get_account_option_description(option);
        auto check = new QCheckBox(this);
        *check_ptr = check;
        add_to_layout(label_text, check);
    };

    name_edit = new QLineEdit(this);
    add_to_layout(tr("Name"), name_edit);

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
    make_attribute_edits(attributes_to_make, top_layout, &attributes);

    make_check(AccountOption_PasswordExpired, &must_change_pass_check);
    // make_check(AccountOption_?, &cannot_change_pass_check);
    make_check(AccountOption_DontExpirePassword, &never_expire_pass_check);
    make_check(AccountOption_Disabled, &account_disabled_check);

    const int button_row = top_layout->rowCount();
    top_layout->addWidget(cancel_button, button_row, 0, Qt::AlignLeft);
    top_layout->addWidget(ok_button, button_row, 1, Qt::AlignRight);

    // PasswordExpired conflicts with CannotChange and DontExpirePassword
    // Prevent the conflicting checks from being set when PasswordExpired is set already and show a message about it
    auto connect_never_expire_conflict =
    [this](QCheckBox *conflict, AccountOption conflicting_option) {
        connect(never_expire_pass_check, &QCheckBox::stateChanged,
            [this, conflict, conflicting_option]() {
                if (checkbox_is_checked(never_expire_pass_check) && checkbox_is_checked(conflict)) {
                    conflict->setCheckState(Qt::Checked);

                    const QString never_expire_text = get_account_option_description(AccountOption_DontExpirePassword);
                    const QString conflicting_text = get_account_option_description(conflicting_option);
                    const QString error = QString(tr("Can't set \"%1\" when \"%2\" is set already.")).arg(conflicting_text, never_expire_text);

                    QMessageBox::warning(this, "Error", error);
                }
            }
            );
    };

    // TODO: make full name auto-generate from first/last
    
    connect_never_expire_conflict(must_change_pass_check, AccountOption_PasswordExpired);
    // connect_never_expire_conflict(must_change_pass_check, AccountOption_?);

    connect(
        this, &QDialog::accepted,
        this, &CreateUserDialog::on_accepted);
}

void CreateUserDialog::on_accepted() {
    const QString name = name_edit->text();

    const QString dn = "CN=" + name + "," + parent_dn;
    const char **classes = get_create_type_classes(CreateType_User);

    AdInterface::instance()->start_batch();
    {
        const AdResult result_add = AdInterface::instance()->object_add(dn, classes);
        QList<AdResult> all_results = { result_add };
        if (result_add.success) {
            const QList<AdResult> attribute_results = apply_attribute_edits(attributes, dn);
            all_results.append(attribute_results);
        }

        if (no_errors(all_results)) {
            Status::instance()->message(QString(tr("Created user - \"%1\"")).arg(name), StatusType_Success);
        } else {
            // Delete object if it was added
            if (result_add.success) {
                AdInterface::instance()->object_delete(dn);
            }

            show_warnings_for_error_results(all_results, this);

            Status::instance()->message(QString(tr("Failed to create user - \"%1\"")).arg(name), StatusType_Error);
        }

    }
    AdInterface::instance()->end_batch();
}

const char **get_create_type_classes(const CreateType type) {
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
}
