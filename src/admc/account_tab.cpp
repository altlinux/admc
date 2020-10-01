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

#include "account_tab.h"
#include "utils.h"
#include "edits/attribute_edit.h"
#include "edits/string_edit.h"
#include "edits/expiry_edit.h"
#include "edits/unlock_edit.h"
#include "edits/account_option_edit.h"
#include "ad_interface.h"

#include <QVBoxLayout>
#include <QPushButton>

// TODO: logon hours, logon computers

// NOTE: https://ldapwiki.com/wiki/MMC%20Account%20Tab

AccountTab::AccountTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    const auto logon_name_edit = new StringEdit(ATTRIBUTE_USER_PRINCIPAL_NAME, CLASS_USER, this);
    edits.append(logon_name_edit);

    edits.append(new UnlockEdit(this));

    QList<AccountOption> options;
    for (int i = 0; i < AccountOption_COUNT; i++) {
        const AccountOption option = (AccountOption) i;
        options.append(option);
    }
    QMap<AccountOption, AccountOptionEdit *> option_edits;
    make_account_option_edits(options, &option_edits, &edits, this);

    edits.append(new ExpiryEdit(this));

    auto edits_layout = new QGridLayout();
    for (auto edit : edits) {
        edit->add_to_layout(edits_layout);
    }

    connect_edits_to_tab(edits, this);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addLayout(edits_layout);
}

bool AccountTab::changed() const {
    return any_edits_changed(edits);
}

bool AccountTab::verify() {
    return verify_attribute_edits(edits, this);
}

void AccountTab::apply() {
    apply_attribute_edits(edits, target(), this);
}

void AccountTab::reload() {
    load_attribute_edits(edits, target());
}

bool AccountTab::accepts_target() const {
    const bool is_user = AdInterface::instance()->is_user(target());

    return is_user;
}
