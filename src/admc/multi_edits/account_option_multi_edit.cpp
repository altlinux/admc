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

#include "multi_edits/account_option_multi_edit.h"

#include "edits/account_option_edit.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>

// TODO: slight duplication of account_option_edit.cpp, but
// not a big deal

AccountOptionMultiEdit::AccountOptionMultiEdit(QList<AttributeMultiEdit *> &edits_out, QObject *parent)
: AttributeMultiEdit(edits_out, parent)
{
    label->setText(tr("Account options:"));

    auto checks_layout = new QGridLayout();

    const QList<AccountOption> option_list = {
        AccountOption_Disabled,
        AccountOption_PasswordExpired,
        AccountOption_DontExpirePassword,
        AccountOption_UseDesKey,
        AccountOption_SmartcardRequired,
        AccountOption_CantDelegate,
        AccountOption_DontRequirePreauth,
    };

    for (const AccountOption &option : option_list) {
        auto check = new QCheckBox();

        check_map[option] = check;

        const int row = checks_layout->rowCount();
        const QString label_text = account_option_string(option);
        checks_layout->addWidget(check, row, 0);
        checks_layout->addWidget(new QLabel(label_text), row, 1);
    }

    auto layout = new QVBoxLayout();
    layout->addLayout(checks_layout);

    auto options_widget = new QWidget();
    options_widget->setLayout(layout);

    options_scroll = new QScrollArea();
    options_scroll->setWidget(options_widget);

    account_option_setup_conflicts(check_map);

    set_enabled(false);
}

void AccountOptionMultiEdit::add_to_layout(QFormLayout *layout) {
    layout->addRow(check_and_label_wrapper, options_scroll);
}

// NOTE: this is slightly inefficient because every account
// option bit is changed with one request, when the whole
// bitmask can be changed at the same time. BUT, do need to
// do this if want to get separate status messages for each
// bit.
bool AccountOptionMultiEdit::apply_internal(AdInterface &ad, const QString &target) {
    const QList<AccountOption> option_change_list =
    [&]() {
        QList<AccountOption> out;

        const AdObject object = ad.search_object(target);

        for (const AccountOption &option : check_map.keys()) {
            QCheckBox *check = check_map[option];

            const bool current_option_state = object.get_account_option(option);
            const bool new_option_state = check->isChecked();
            const bool option_changed = (new_option_state != current_option_state);
            if (option_changed) {
                out.append(option);
            }
        }
    
        return out;
    }();

    bool total_success = true;

    for (const AccountOption &option : option_change_list) {
        QCheckBox *check = check_map[option];
        const bool option_is_set = check->isChecked();

        const bool success = ad.user_set_account_option(target, option, option_is_set);

        if (!success) {
            total_success = false;
        }
    }

    return total_success;
}

void AccountOptionMultiEdit::set_enabled(const bool enabled) {
    if (!enabled) {
        for (QCheckBox *check : check_map.values()) {
            check->setChecked(false);
        }
    }

    options_scroll->setEnabled(enabled);
}
