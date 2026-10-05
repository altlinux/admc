/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2021-2026 BaseALT Ltd.
 * Copyright (C) 2021-2022 Dmitry Degtyarev
 * Copyright (C) 2023 Ivan A. Melnikov
 * Copyright (C) 2024-2025 Semyon Knyazev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>

#include "adldap.h"
#include "core/line_edit_utils.h"
#include "core/settings.h"
#include "core/utils.h"
#include "ui/attribute_edit/account_option_edit.h"
#include "ui/attribute_edit/password_edit.h"
#include "ui/attribute_edit/sam_name_edit.h"
#include "ui/attribute_edit/string_edit.h"
#include "ui/attribute_edit/upn_edit.h"
#include "ui/create_object_helper.h"
#include "ui/dialog/create/ui_user.h"
#include "ui/dialog/create/user.h"

void CreateUserDialog::setup_field_validation() {
    field_to_error_map = {
        { ui->first_name_edit, ui->first_name_error },
        { ui->upn_prefix_edit, ui->upn_prefix_error },
        { ui->sam_name_edit, ui->sam_name_error}
    };

    QPlainTextEdit *error_label;
    QPalette palette = this->palette();
    palette.setColor(QPalette::Base,
                     this->window()->palette().color(QPalette::Window));
    for (auto it = field_to_error_map.begin(); it != field_to_error_map.end();
         it++) {
        error_label = it.value();
        error_label->setPalette(palette);
        error_label->setFrameShape(QFrame::NoFrame);
    }

    const QList<QLineEdit *> field_list = {
        ui->first_name_edit,
        ui->last_name_edit,
        ui->middle_name_edit,
        ui->upn_prefix_edit,
        ui->sam_name_edit,
    };
    for (const auto &field : field_list) {
        connect(field, &QLineEdit::textEdited,
                [=](const QString &text) {
                    Q_UNUSED(text);
                    validate_fields(field);
                });
    }
}

CreateUserDialog::CreateUserDialog(AdInterface &ad,
                                   const QString &parent_dn,
                                   const QString &user_class,
                                   QWidget *parent)
    : CreateObjectDialog(parent)
{
    ui = new Ui::CreateUserDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    bool show_middle_name = false;
    const QVariant show_middle_name_variant =
        settings_get_variant(SETTING_show_middle_name_when_creating);
    if (! show_middle_name_variant.isNull()) {
        show_middle_name = show_middle_name_variant.toBool();
    }
    ui->middle_name_edit->setVisible(show_middle_name);
    ui->middle_name_label->setVisible(show_middle_name);

    auto first_name_edit =
        new StringEdit(ui->first_name_edit, ATTRIBUTE_FIRST_NAME, this);
    auto last_name_edit =
        new StringEdit(ui->last_name_edit, ATTRIBUTE_LAST_NAME, this);
    auto initials_edit =
        new StringEdit(ui->initials_edit, ATTRIBUTE_INITIALS, this);
    auto sam_name_edit =
        new SamNameEdit(ui->sam_name_edit, ui->sam_name_domain_edit, this);
    auto password_edit =
        new PasswordEdit(ui->password_main_edit, ui->password_confirm_edit,
                         ui->show_password_check, this);
    auto middle_name_edit =
        new StringEdit(ui->middle_name_edit, ATTRIBUTE_MIDDLE_NAME, this);

    auto upn_edit = new UpnEdit(ui->upn_prefix_edit, ui->upn_suffix_edit, this);
    upn_edit->init_suffixes(ad);

    const QHash<AccountOption, QCheckBox *> check_map = {
        { AccountOption_PasswordExpired, ui->must_change_pass_check },
        { AccountOption_CantChangePassword, ui->cant_change_pass_check },
        { AccountOption_DontExpirePassword, ui->dont_expire_pass_check },
        { AccountOption_Disabled, ui->disabled_check },
    };

    QList<AttributeEdit *> option_edit_list;

    for (const AccountOption &option : check_map.keys()) {
        QCheckBox *check = check_map[option];
        auto edit = new AccountOptionEdit(check, option, this);
        option_edit_list.append(edit);
    }

    account_option_setup_conflicts(check_map);

    setup_field_validation();

    const QList<QLineEdit *> required_list = {
        ui->name_edit,
        ui->first_name_edit,
        ui->sam_name_edit,
    };

    QList<AttributeEdit *> edit_list = {
        first_name_edit,
        last_name_edit,
        initials_edit,
        sam_name_edit,
        password_edit,
        upn_edit,
        middle_name_edit,
    };

    edit_list.append(option_edit_list);

    helper = new CreateObjectHelper(ui->name_edit, ui->button_box, edit_list,
                                    required_list, user_class, parent_dn, this);

    if (user_class != CLASS_USER) {
        setWindowTitle(QString(tr("Create %1")).arg(user_class));
    }

    settings_setup_dialog_geometry(SETTING_create_user_dialog_geometry, this);
}

void CreateUserDialog::validate_fields(QLineEdit *changed_field) {
    if ((changed_field == ui->first_name_edit) ||
        (changed_field == ui->last_name_edit) ||
        (changed_field == ui->middle_name_edit)) {
        line_edit_full_name_autofill(ui->first_name_edit, ui->last_name_edit,
                                     ui->middle_name_edit, ui->name_edit);
    }

    if (changed_field == ui->upn_prefix_edit) {
        ui->sam_name_edit->setText(ui->upn_prefix_edit->text());
    }

    bool valid = true;
    QLineEdit *line_edit;
    QPlainTextEdit *error_label;
    for (auto it = field_to_error_map.begin(); it != field_to_error_map.end();
         it++) {
        line_edit = it.key();
        error_label = it.value();
        bool field_valid = is_object_name_valid(line_edit->text());
        error_label->setVisible(! field_valid);
        if (field_valid) {
            error_label->setPlainText("");
        } else {
            error_label->setPlainText(
                tr("Illegal characters found: # , + \" \\ < > ; = "
                   "leading/trailing space or a leading question mark"));
        }
        valid = valid && field_valid;
    }
    helper->set_input_valid(valid);
    ui->button_box->button(QDialogButtonBox::Ok)->setDisabled(! valid);
}

CreateUserDialog::~CreateUserDialog() {
    delete ui;
}

void CreateUserDialog::accept() {
    const bool accepted = helper->accept();

    if (accepted) {
        QDialog::accept();
    }
}

QString CreateUserDialog::get_created_dn() const {
    return helper->get_created_dn();
}
