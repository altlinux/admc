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

#include "attribute_edits/account_option_edit.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QMap>

AccountOptionEdit::AccountOptionEdit(QCheckBox *check_arg, const AccountOption option_arg, QObject *parent)
: AttributeEdit(parent) {
    option = option_arg;
    check = check_arg;

    connect(
        check, &QCheckBox::stateChanged,
        this, &AttributeEdit::edited);
}

void AccountOptionEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    const bool option_is_set = object.get_account_option(option, g_adconfig);
    check->setChecked(option_is_set);
}

bool AccountOptionEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool new_value = check->isChecked();
    const bool success = ad.user_set_account_option(dn, option, new_value);

    return success;
}

// PasswordExpired conflicts with (DontExpirePassword and
// CantChangePassword) When PasswordExpired is set, the
// other two can't be set When any of the other two are set,
// PasswordExpired can't be set Implement this by connecting
// to state changes of all options and resetting to previous
// state if state transition is invalid
void account_option_setup_conflicts(const QHash<AccountOption, QCheckBox *> &check_map) {
    auto setup_conflict = [&](const AccountOption subject_option, const AccountOption blocker_option) {
        if (!check_map.contains(subject_option) || !check_map.contains(blocker_option)) {
            return;
        }

        QCheckBox *subject = check_map[subject_option];
        QCheckBox *blocker = check_map[blocker_option];

        QObject::connect(
            subject, &QCheckBox::clicked,
            blocker,
            [subject, blocker, subject_option, blocker_option]() {
                const bool conflict = (subject->isChecked() && blocker->isChecked());
                if (conflict) {
                    subject->setChecked(false);

                    const QString subject_name = account_option_string(subject_option);
                    const QString blocker_name = account_option_string(blocker_option);
                    const QString error = QString(QObject::tr("Can't set \"%1\" when \"%2\" is set.")).arg(subject_name, blocker_name);
                    message_box_warning(blocker, QObject::tr("Error"), error);
                }
            });
    };

    const QList<AccountOption> other_two_options = {
        AccountOption_DontExpirePassword,
        AccountOption_CantChangePassword,
    };

    for (auto other_option : other_two_options) {
        setup_conflict(AccountOption_PasswordExpired, other_option);
        setup_conflict(other_option, AccountOption_PasswordExpired);
    }
}
