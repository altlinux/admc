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

#include "multi_tabs/account_multi_tab.h"
#include "multi_tabs/ui_account_multi_tab.h"

#include "adldap.h"
#include "attribute_edits/account_option_multi_edit.h"
#include "attribute_edits/expiry_edit.h"
#include "attribute_edits/string_edit.h"
#include "attribute_edits/upn_multi_edit.h"

#include <QHash>

AccountMultiTab::AccountMultiTab(AdInterface &ad, QList<AttributeEdit *> *edit_list, QHash<AttributeEdit *, QCheckBox *> *check_map, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::AccountMultiTab();
    ui->setupUi(this);

    auto upn_edit = new UpnMultiEdit(ui->upn_edit, ad, this);
    const QHash<AccountOption, QCheckBox *> option_to_check_map = {
        {AccountOption_Disabled, ui->option_disabled},
        {AccountOption_PasswordExpired, ui->option_pass_expired},
        {AccountOption_DontExpirePassword, ui->option_dont_expire_pass},
        {AccountOption_UseDesKey, ui->option_use_des_key},
        {AccountOption_SmartcardRequired, ui->option_smartcard},
        {AccountOption_CantDelegate, ui->option_cant_delegate},
        {AccountOption_DontRequirePreauth, ui->option_dont_require_kerb},
    };
    auto options_edit = new AccountOptionMultiEdit(option_to_check_map, this);
    auto expiry_edit = new ExpiryEdit(ui->expiry_edit, this);

    edit_list->append({
        upn_edit,
        options_edit,
        expiry_edit,
    });

    check_map->insert(upn_edit, ui->upn_check);
    check_map->insert(options_edit, ui->options_check);
    check_map->insert(expiry_edit, ui->expiry_check);
}

AccountMultiTab::~AccountMultiTab() {
    delete ui;
}
