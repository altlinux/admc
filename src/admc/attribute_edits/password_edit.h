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

#ifndef PASSWORD_EDIT_H
#define PASSWORD_EDIT_H

#include "attribute_edits/attribute_edit.h"

class QLineEdit;
class QCheckBox;

class PasswordEdit final : public AttributeEdit {
    Q_OBJECT
public:
    PasswordEdit(QLineEdit *edit_arg, QLineEdit *confirm_edit_arg, QCheckBox *show_password_check, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;
    bool verify(AdInterface &ad, const QString &dn) const override;
    bool apply(AdInterface &ad, const QString &dn) const override;

    QLineEdit *get_edit() const;
    QLineEdit *get_confirm_edit() const;

private:
    QLineEdit *edit;
    QLineEdit *confirm_edit;

    void on_show_password_check(bool checked);
};

#endif /* PASSWORD_EDIT_H */
