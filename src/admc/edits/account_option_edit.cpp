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

#include "edits/account_option_edit.h"
#include "ad_interface.h"
#include "ad_utils.h"
#include "utils.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>
#include <QDateTimeEdit>
#include <QMessageBox>

void AccountOptionEdit::make_many(const QList<AccountOption> options, QMap<AccountOption, AccountOptionEdit *> *option_edits_out, QList<AttributeEdit *> *edits_out, QWidget *parent) {
    QMap<AccountOption, AccountOptionEdit *> option_edits;

    for (auto option : options) {
        auto edit = new AccountOptionEdit(option, edits_out, parent);
        option_edits.insert(option, edit);
        option_edits_out->insert(option, edit);
    }

    // PasswordExpired conflicts with (DontExpirePassword and CantChangePassword)
    // When PasswordExpired is set, the other two can't be set
    // When any of the other two are set, PasswordExpired can't be set
    // Implement this by connecting to state changes of all options and
    // resetting to previous state if state transition is invalid
    auto setup_conflict =
    [parent, option_edits](const AccountOption subject, const AccountOption blocker) {
        QCheckBox *subject_check = option_edits[subject]->check;
        QCheckBox *blocker_check = option_edits[blocker]->check;

        QObject::connect(subject_check, &QCheckBox::stateChanged,
            [subject, blocker, subject_check, blocker_check, parent]() {
                if (subject_check->isChecked() && blocker_check->isChecked()) {
                    subject_check->setCheckState(Qt::Unchecked);

                    const QString subject_name = account_option_string(subject);
                    const QString blocker_name = account_option_string(blocker);
                    const QString error = QString(QObject::tr("Can't set \"%1\" when \"%2\" is set.")).arg(blocker_name, subject_name);
                    QMessageBox::warning(nullptr, QObject::tr("Error"), error);
                }
            }
            );
    };

    // NOTE: only setup conflicts for options that exist
    // TODO: AccountOption_CantChangePassword. Once security descriptor manipulation is implemented, it should be evident how to do this. See link: https://docs.microsoft.com/en-us/windows/win32/adsi/modifying-user-cannot-change-password-ldap-provider?redirectedfrom=MSDN
    if (options.contains(AccountOption_PasswordExpired)) {
        const QList<AccountOption> other_two_options = {
            AccountOption_DontExpirePassword,
            // AccountOption_CantChangePassword,
        };

        for (auto other_option : other_two_options) {
            if (options.contains(other_option)) {
                setup_conflict(AccountOption_PasswordExpired, other_option);
                setup_conflict(other_option, AccountOption_PasswordExpired);
            }
        }
    }
}

AccountOptionEdit::AccountOptionEdit(const AccountOption option_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent)
{
    option = option_arg;
    check = new QCheckBox();

    QObject::connect(
        check, &QCheckBox::stateChanged,
        [this]() {
            emit edited();
        });
}

void AccountOptionEdit::load_internal(const AdObject &object) {
    const bool option_is_set = object.get_account_option(option);
    check->setChecked(option_is_set);
}

void AccountOptionEdit::set_read_only(const bool read_only) {
    check->setDisabled(read_only);
}

void AccountOptionEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = account_option_string(option) + ":";
    layout->addRow(label_text, check);
}

bool AccountOptionEdit::apply(const QString &dn) const {
    const bool new_value = check->isChecked();
    const bool success = AD()->user_set_account_option(dn, option, new_value);

    return success;
}
