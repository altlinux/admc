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

#ifndef STRING_EDIT_H
#define STRING_EDIT_H

#include "attribute_edit.h"

#include <QString>
#include <QMap>
#include <QList>

class QLineEdit;

class StringEdit final : public AttributeEdit {
Q_OBJECT
public:
    QLineEdit *edit;

    StringEdit(const QString &attribute_arg);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();
    void set_read_only(EditReadOnly read_only_arg);

private:
    QString attribute;
    QString original_value;
};

void make_string_edits(const QList<QString> attributes, QMap<QString, StringEdit *> *edits_out);
void autofill_full_name(QMap<QString, StringEdit *> string_edits);
void autofill_sama_name(StringEdit *sama_edit, StringEdit *name_edit);

#endif /* STRING_EDIT_H */
