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

#ifndef STRING_OTHER_EDIT_H
#define STRING_OTHER_EDIT_H

#include "edits/attribute_edit.h"

class StringEdit;
class QPushButton;
class QLineEdit;

/**
 * Edit for attributes which have "other" version which is
 * multi-valued. For example "telephone" and
 * "otherTelephone". Contains a StringEdit for the attribute
 * itself and an "Other" button next to it which opens a
 * dialog through which other values are edited.
 */

class StringOtherEdit final : public AttributeEdit {
    Q_OBJECT
public:
    StringOtherEdit(QLineEdit *line_edit, QPushButton *other_button, const QString &main_attribute_arg, const QString &other_attribute_arg, QList<AttributeEdit *> *edits_out, QObject *parent);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    StringEdit *main_edit;
    QPushButton *other_button;

    const QString other_attribute;
    QList<QByteArray> other_values;
};

#endif /* STRING_OTHER_EDIT_H */
