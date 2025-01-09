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

#include "attribute_edits/account_option_multi_edit.h"

#include "adldap.h"
#include "attribute_edits/account_option_edit.h"
#include "globals.h"

#include <QCheckBox>

AccountOptionMultiEdit::AccountOptionMultiEdit(const QHash<AccountOption, QCheckBox *> &check_map_arg, QObject *parent)
: AttributeEdit(parent) {
    check_map = check_map_arg;

    account_option_setup_conflicts(check_map);
}

// NOTE: this is slightly inefficient because every account
// option bit is changed with one request, when the whole
// bitmask can be changed at the same time. BUT, do need to
// do this if want to get separate status messages for each
// bit.
bool AccountOptionMultiEdit::apply(AdInterface &ad, const QString &target) const {
    const QList<AccountOption> option_change_list = [&]() {
        QList<AccountOption> out;

        const AdObject object = ad.search_object(target);

        for (const AccountOption &option : check_map.keys()) {
            QCheckBox *check = check_map[option];

            const bool current_option_state = object.get_account_option(option, g_adconfig);
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
    for (QCheckBox *check : check_map.values()) {
        check->setEnabled(enabled);
    }
}
