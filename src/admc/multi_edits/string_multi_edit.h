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

#ifndef STRING_MULTI_EDIT_H
#define STRING_MULTI_EDIT_H

#include "multi_edits/attribute_multi_edit.h"

class QLineEdit;

/**
 * Edit for editing string attributes of multiple objects.
 */

class StringMultiEdit : public AttributeMultiEdit {
    Q_OBJECT
public:
    StringMultiEdit(const QString &attribute_arg, QList<AttributeMultiEdit *> &edits_out, QObject *parent);

    DECL_ATTRIBUTE_MULTI_EDIT_VIRTUALS();

private:
    QLineEdit *edit;
    QString attribute;
};

#endif /* STRING_MULTI_EDIT_H */
