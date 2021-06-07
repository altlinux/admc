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

#ifndef COUNTRY_MULTI_EDIT_H
#define COUNTRY_MULTI_EDIT_H

#include "multi_edits/attribute_multi_edit.h"

class CountryWidget;

/**
 * Edit for editing string attributes of multiple objects.
 */

class CountryMultiEdit : public AttributeMultiEdit {
Q_OBJECT
public:
    CountryMultiEdit(QList<AttributeMultiEdit *> &edits_out, QObject *parent);

    DECL_ATTRIBUTE_MULTI_EDIT_VIRTUALS();

private:
    CountryWidget *country_widget;
};

#endif /* COUNTRY_MULTI_EDIT_H */
