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

#ifndef ACCOUNT_OPTION_EDIT_H
#define ACCOUNT_OPTION_EDIT_H

#include "edits/attribute_edit.h"
#include "ad_interface.h"

class QCheckBox;
class QWidget;

class AccountOptionEdit final : public AttributeEdit {
Q_OBJECT
public:
    QCheckBox *check;

    AccountOptionEdit(const AccountOption option_arg, QObject *parent);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    AccountOption option;
    bool original_value;
};

void make_account_option_edits(const QList<AccountOption> options, QMap<AccountOption, AccountOptionEdit *> *option_edits_out, QList<AttributeEdit *> *edits_out, QWidget *parent);

#endif /* ACCOUNT_OPTION_EDIT_H */
