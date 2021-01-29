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
    static void make_many(const QList<QString> attributes, const QString &objectClass, QList<AttributeEdit *> *edits_out, QObject *parent);

    StringEdit(const QString &attribute_arg, const QString &objectClass_arg, QList<AttributeEdit *> *edits_out, QObject *parent);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

    QString get_input() const;
    void set_input(const QString &value);
    bool is_empty() const;

private:
    QLineEdit *edit;
    QString attribute;
    QString objectClass;

    friend class StringOtherEdit;
};

#endif /* STRING_EDIT_H */
