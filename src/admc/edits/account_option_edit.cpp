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
#include "utils.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QDateTimeEdit>
#include <QMessageBox>
#include <QLabel>

void make_account_option_edits(const AdObject &object, const QList<AccountOption> options, QMap<AccountOption, AccountOptionEdit *> *option_edits_out, QList<AttributeEdit *> *edits_out, QWidget *parent) {
    QMap<AccountOption, AccountOptionEdit *> option_edits;

    for (auto option : options) {
        auto edit = new AccountOptionEdit(object, option, parent);
        option_edits.insert(option, edit);
        option_edits_out->insert(option, edit);
        edits_out->append(edit);
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
                if (checkbox_is_checked(subject_check) && checkbox_is_checked(blocker_check)) {
                    subject_check->setCheckState(Qt::Unchecked);

                    const QString subject_name = account_option_string(subject);
                    const QString blocker_name = account_option_string(blocker);
                    const QString error = QString(QObject::tr("Can't set \"%1\" when \"%2\" is set.")).arg(blocker_name, subject_name);
                    QMessageBox::warning(parent, QObject::tr("Error"), error);
                }
            }
            );
    };

    // NOTE: only setup conflicts for options that exist
    if (options.contains(AccountOption_PasswordExpired)) {
        const QList<AccountOption> other_two_options = {
            AccountOption_DontExpirePassword,
            // TODO: AccountOption_CantChangePassword
        };

        for (auto other_option : other_two_options) {
            if (options.contains(other_option)) {
                setup_conflict(AccountOption_PasswordExpired, other_option);
                setup_conflict(other_option, AccountOption_PasswordExpired);
            }
        }
    }
}

AccountOptionEdit::AccountOptionEdit(const AdObject &object, const AccountOption option_arg, QObject *parent)
: AttributeEdit(parent)
{
    option = option_arg;
    check = new QCheckBox();

    original_value = object.get_account_option(option);
    checkbox_set_checked(check, original_value);

    QObject::connect(
        check, &QCheckBox::stateChanged,
        [this]() {
            emit edited();
        });
}

void AccountOptionEdit::set_read_only(const bool read_only) {
    check->setDisabled(read_only);
}

void AccountOptionEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = account_option_string(option) + ":";
    const auto label = new QLabel(label_text);

    connect_changed_marker(this, label);
    append_to_grid_layout_with_label(layout, label, check);
}

bool AccountOptionEdit::verify_input(QWidget *parent) {
    return true;
}

bool AccountOptionEdit::changed() const {
    const bool new_value = checkbox_is_checked(check);
    return (new_value != original_value);
}

bool AccountOptionEdit::apply(const QString &dn) {
    const bool new_value = checkbox_is_checked(check);
    const bool success = AD()->user_set_account_option(dn, option, new_value);

    return success;
}
