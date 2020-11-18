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

#ifndef STRING_MULTI_EDIT_H
#define STRING_MULTI_EDIT_H

#include "attribute_edit.h"

#include <QString>
#include <QByteArray>
#include <QList>

class QLabel;
class StringEdit;
class QPushButton;

// Edit for attributes which have "other" version which is
// multi-valued. For example "telephone" and "otherTelephone".
// Contains a StringEdit for the attribute itself and an "Other"
// button next to it which opens a dialog through which other
// values are edited.

class StringOtherEdit final : public AttributeEdit {
Q_OBJECT
public:
    StringOtherEdit(const QString &main_attribute_arg, const QString &other_attribute_arg, const QString &object_class, QObject *parent, QList<AttributeEdit *> *edits_out = nullptr);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    StringEdit *main_edit;
    QPushButton *other_button;

    const QString other_attribute;
    QList<QByteArray> other_values;
};

#endif /* STRING_MULTI_EDIT_H */
