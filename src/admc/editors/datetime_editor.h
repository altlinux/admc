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

#ifndef DATETIME_EDITOR_H
#define DATETIME_EDITOR_H

#include "editors/attribute_editor.h"

class QDateTimeEdit;

class DateTimeEditor final : public AttributeEditor {
Q_OBJECT

public:
    DateTimeEditor(const QString attribute, const QList<QByteArray> values, QWidget *parent);

    QList<QByteArray> get_new_values() const;

private:
    QDateTimeEdit *edit;
};

#endif /* DATETIME_EDITOR_H */
