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

#include "multi_tabs/account_multi_tab.h"
#include "multi_tabs/ui_account_multi_tab.h"

#include "adldap.h"
#include "attribute_multi_edits/account_option_multi_edit.h"
#include "attribute_multi_edits/expiry_multi_edit.h"
#include "attribute_multi_edits/string_multi_edit.h"
#include "attribute_multi_edits/upn_multi_edit.h"

AccountMultiTab::AccountMultiTab(QList<AttributeMultiEdit *> *edit_list, AdInterface &ad, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::AccountMultiTab();
    ui->setupUi(this);

    new UpnMultiEdit(ui->upn_edit, ui->upn_check, edit_list, ad, this);
    const QHash<AccountOption, QCheckBox *> check_map = {
        {AccountOption_Disabled, ui->option_disabled},
        {AccountOption_PasswordExpired, ui->option_pass_expired},
        {AccountOption_DontExpirePassword, ui->option_dont_expire_pass},
        {AccountOption_UseDesKey, ui->option_use_des_key},
        {AccountOption_SmartcardRequired, ui->option_smartcard},
        {AccountOption_CantDelegate, ui->option_cant_delegate},
        {AccountOption_DontRequirePreauth, ui->option_dont_require_kerb},
    };
    new AccountOptionMultiEdit(check_map, ui->options_check, edit_list, this);
    new ExpiryMultiEdit(ui->expiry_edit, ui->expiry_check, edit_list, this);
}

AccountMultiTab::~AccountMultiTab() {
    delete ui;
}
