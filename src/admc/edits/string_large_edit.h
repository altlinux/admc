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

#ifndef STRING_LARGE_EDIT_H
#define STRING_LARGE_EDIT_H

/**
 * Edit for large, multi-line strings. Uses QPlainTextEdit
 * instead of QLineEdit.
 */

#include "attribute_edit.h"

#include <QString>
#include <QList>

class QPlainTextEdit;

class StringLargeEdit final : public AttributeEdit {
Q_OBJECT
public:
    StringLargeEdit(const QString &attribute_arg, const QString &objectClass_arg, QList<AttributeEdit *> *edits_out, QObject *parent);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    QPlainTextEdit *edit;
    QString attribute;
    QString objectClass;
};

#endif /* STRING_LARGE_EDIT_H */
