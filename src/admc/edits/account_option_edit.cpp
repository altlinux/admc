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

#include "edits/account_option_edit.h"

#include "adldap.h"
#include "utils.h"
#include "globals.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>
#include <QDateTimeEdit>
#include <QMessageBox>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>

void AccountOptionEdit::make_many(const QList<AccountOption> options, QMap<AccountOption, AccountOptionEdit *> *option_edits_out, QList<AttributeEdit *> *edits_out, QWidget *parent) {
    QHash<AccountOption, QCheckBox *> check_map;

    for (auto option : options) {
        auto edit = new AccountOptionEdit(option, edits_out, parent);
        check_map[option] = edit->check;
        option_edits_out->insert(option, edit);
    }

    account_option_setup_conflicts(check_map);
}

QWidget *AccountOptionEdit::layout_many(const QList<AccountOption> &options, const QMap<AccountOption, AccountOptionEdit *> &option_edits) {
    auto checks_layout = new QGridLayout();
    for (const auto option : options) {
        auto edit = option_edits[option];

        const int row = checks_layout->rowCount();
        const QString label_text = account_option_string(edit->option);
        checks_layout->addWidget(edit->check, row, 0);
        checks_layout->addWidget(new QLabel(label_text), row, 1);
    }

    auto layout = new QVBoxLayout();
    layout->addWidget(new QLabel(tr("Account options:")));
    layout->addLayout(checks_layout);

    auto options_widget = new QWidget();
    options_widget->setLayout(layout);

    auto options_scroll = new QScrollArea();
    options_scroll->setWidget(options_widget);

    return options_scroll;
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

void AccountOptionEdit::load_internal(AdInterface &ad, const AdObject &object) {
    const bool option_is_set = object.get_account_option(option, g_adconfig);
    check->setChecked(option_is_set);
}

void AccountOptionEdit::set_read_only(const bool read_only) {
    check->setDisabled(read_only);
}

void AccountOptionEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = account_option_string(option) + ":";
    layout->addRow(label_text, check);
}

bool AccountOptionEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool new_value = check->isChecked();
    const bool success = ad.user_set_account_option(dn, option, new_value);

    return success;
}

void AccountOptionEdit::set_checked(const bool checked) {
    check->setChecked(checked);
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
            subject, &QCheckBox::stateChanged,
            [&, subject, blocker]() {
                const bool conflict = (subject->isChecked() && blocker->isChecked());
                if (conflict) {
                    subject->setChecked(false);

                    const QString subject_name = account_option_string(subject_option);
                    const QString blocker_name = account_option_string(blocker_option);
                    const QString error = QString(QObject::tr("Can't set \"%1\" when \"%2\" is set.")).arg(blocker_name, subject_name);
                    QMessageBox::warning(blocker, QObject::tr("Error"), error);
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
