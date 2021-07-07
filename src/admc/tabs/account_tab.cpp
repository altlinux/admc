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

#include "tabs/account_tab.h"
#include "edits/account_option_edit.h"
#include "edits/expiry_edit.h"
#include "edits/string_edit.h"
#include "edits/unlock_edit.h"
#include "edits/upn_edit.h"
#include "edits/logon_hours_edit.h"
#include "edits/logon_computers_edit.h"

#include <QFormLayout>

// TODO: logon computers

// NOTE: the "can't change password" checkbox does not
// affect the permission in the security tab, even though
// they control the same thing. And vice versa. Too
// complicated to implement so this is a WONTFIX.

AccountTab::AccountTab(AdInterface &ad) {
    auto upn_edit = new UpnEdit(&edits, ad, this);

    auto unlock_edit = new UnlockEdit(&edits, this);

    auto expiry_edit = new ExpiryEdit(&edits, this);

    auto logon_hours_edit = new LogonHoursEdit(&edits, this);

    auto logon_computers_edit = new LogonComputersEdit(&edits, this);

    QList<AccountOption> options;
    for (int i = 0; i < AccountOption_COUNT; i++) {
        const AccountOption option = (AccountOption) i;
        options.append(option);
    }
    QMap<AccountOption, AccountOptionEdit *> option_edits;
    AccountOptionEdit::make_many(options, &option_edits, &edits, this);
    QWidget *options_widget = AccountOptionEdit::layout_many(options, option_edits);

    edits_connect_to_tab(edits, this);

    auto edits_layout = new QFormLayout();

    const QList<AttributeEdit *> top_edits = {
        upn_edit,
        unlock_edit,
        expiry_edit,
        logon_hours_edit,
        logon_computers_edit,
    };
    edits_add_to_layout(top_edits, edits_layout);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(edits_layout);
    layout->addWidget(options_widget);
}
