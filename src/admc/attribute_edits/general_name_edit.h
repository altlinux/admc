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

#ifndef GENERAL_NAME_EDIT_H
#define GENERAL_NAME_EDIT_H

/**
 * Edit for displaying name of object in a label. Used
 * in general tabs of the properties dialog
 */

#include "attribute_edits/attribute_edit.h"

class QLabel;

class GeneralNameEdit final : public AttributeEdit {
    Q_OBJECT
public:
    GeneralNameEdit(QLabel *label, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;

private:
    QLabel *label;
};

#endif /* GENERAL_NAME_EDIT_H */
