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

    StringEdit(const AdObject &object, const QString &attribute_arg, const QString &objectClass_arg, QObject *parent);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    QString attribute;
    QString objectClass;
    QString original_value;
};

void make_string_edits(const AdObject &object, const QList<QString> attributes, const QString &objectClass, QMap<QString, StringEdit *> *string_edits_out, QList<AttributeEdit *> *edits_out, QObject *parent);
void setup_string_edit_autofills(const QMap<QString, StringEdit *> string_edits, const StringEdit *name_edit = nullptr);

#endif /* STRING_EDIT_H */
