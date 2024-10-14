/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#ifndef COMPUTER_SAM_NAME_EDIT_H
#define COMPUTER_SAM_NAME_EDIT_H

#include "attribute_edits/attribute_edit.h"

class QLineEdit;

class ComputerSamNameEdit final : public AttributeEdit {
    Q_OBJECT
public:
    ComputerSamNameEdit(QLineEdit *edit, QLineEdit *domain_edit, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;
    bool verify(AdInterface &ad, const QString &dn) const override;
    bool apply(AdInterface &ad, const QString &dn) const override;

    void set_enabled(const bool enabled) override;

private:
    QLineEdit *edit;
};

#endif /* COMPUTER_SAM_NAME_EDIT_H */
