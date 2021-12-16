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

#ifndef ACCOUNT_OPTION_MULTI_EDIT_H
#define ACCOUNT_OPTION_MULTI_EDIT_H

#include "attribute_multi_edits/attribute_multi_edit.h"

#include "ad_defines.h"

#include <QHash>

class AccountOptionMultiEdit final : public AttributeMultiEdit {
    Q_OBJECT
public:
    AccountOptionMultiEdit(const QHash<AccountOption, QCheckBox *> &check_map_arg, QCheckBox *check, QList<AttributeMultiEdit *> *edit_list, QObject *parent);

    bool apply(AdInterface &ad, const QString &target) override;
    void set_enabled(const bool enabled) override;

private:
    QHash<AccountOption, QCheckBox *> check_map;
};

#endif /* ACCOUNT_OPTION_MULTI_EDIT_H */
