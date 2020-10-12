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
class QLabel;

class StringEdit final : public AttributeEdit {
Q_OBJECT
public:

    static void setup_autofill(const QList<StringEdit *> &string_edits);

    StringEdit(const QString &attribute_arg, const QString &objectClass_arg, QObject *parent);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

    QString get_input() const;

private:
    QLineEdit *edit;
    QLabel *label;
    QString attribute;
    QString objectClass;
    QString original_value;

    friend class StringOtherEdit;
};

// Convenience f-ns that insert string edits into an edits list
// and a map for you.
StringEdit *make_string_edit(const QString &attribute, const QString &objectClass, QObject *parent, QMap<QString, StringEdit *> *map_out, QList<AttributeEdit *> *edits_out);
void make_string_edits(const QList<QString> attributes, const QString &objectClass, QObject *parent, QMap<QString, StringEdit *> *map_out, QList<AttributeEdit *> *edits_out);

#endif /* STRING_EDIT_H */
