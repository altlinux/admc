/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#include "tabs/laps_v2_tab.h"
#include "tabs/ui_laps_v2_tab.h"

#include "adldap.h"
#include "attribute_edits/laps_expiry_edit.h"
#include "attribute_edits/laps_encrypted_attribute_edit.h"
#include "attribute_edits/string_edit.h"

#include "utils.h"

#include <QClipboard>

#define USERNAME "n"
#define PASSWORD "p"

LAPSV2Tab::LAPSV2Tab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    dialog_has_been_shown = false;

    ui = new Ui::LAPSV2Tab();
    ui->setupUi(this);

    auto laps_edit = new LAPSExpiryEdit(ui->expiration_datetimeedit, ui->expire_now_button, ATTRIBUTE_LAPS_V2_EXPIRATION_TIME, this);
    auto user_name_edit = new LAPSEncryptedAttributeEdit(ui->admin_name_lineedit, ATTRIBUTE_LAPS_V2_ENCRYPTED_PASSWORD, USERNAME, this);
    auto pass_word_edit = new LAPSEncryptedAttributeEdit(ui->admin_password_lineedit, ATTRIBUTE_LAPS_V2_ENCRYPTED_PASSWORD, PASSWORD, this);

    edit_list->append({
        laps_edit,
        user_name_edit,
        pass_word_edit,
    });

    connect(user_name_edit, &LAPSEncryptedAttributeEdit::show_error_dialog, this, &LAPSV2Tab::on_show_error_dialog);
    connect(pass_word_edit, &LAPSEncryptedAttributeEdit::show_error_dialog, this, &LAPSV2Tab::on_show_error_dialog);
}

LAPSV2Tab::~LAPSV2Tab() {
    delete ui;
}

void LAPSV2Tab::on_show_password_button_toggled(bool checked)
{
    ui->admin_password_lineedit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}


void LAPSV2Tab::on_copy_password_button_clicked()
{
    QApplication::clipboard()->setText(ui->admin_password_lineedit->text());
}


void LAPSV2Tab::on_expiration_datetimeedit_dateTimeChanged(const QDateTime &dateTime)
{
    ui->current_password_expiration_lineedit->setText(dateTime.toString());
}

void LAPSV2Tab::on_show_error_dialog()
{
    if (!dialog_has_been_shown)
    {
        dialog_has_been_shown = true;

        message_box_warning(this, tr("LAPS data decoding failed!"), tr("Check access rights to LAPS attributes!"));
    }
}

