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

#ifndef OCTET_EDITOR_H
#define OCTET_EDITOR_H

#include "editors/attribute_editor.h"

class QPlainTextEdit;
class QComboBox;

enum OctetDisplayFormat {
    OctetDisplayFormat_Hexadecimal,
    OctetDisplayFormat_Binary,
    OctetDisplayFormat_Decimal,
    OctetDisplayFormat_Octal,
};

class OctetEditor final : public AttributeEditor {
Q_OBJECT

public:
    OctetEditor(const QString attribute, const QList<QByteArray> values, QWidget *parent);

    QList<QByteArray> get_new_values() const;
    void accept() override;

    private slots:
    void on_format_combo();

private:
    QPlainTextEdit *edit;
    QComboBox *format_combo;

    bool check_input(const OctetDisplayFormat format);
};

QComboBox *make_format_combo();
OctetDisplayFormat current_format(QComboBox *format_combo);
QString bytes_to_string(const QByteArray bytes, const OctetDisplayFormat format);
QByteArray string_to_bytes(const QString string, const OctetDisplayFormat format);
QString string_change_format(const QString old_string, const OctetDisplayFormat old_format, const OctetDisplayFormat new_format);

#endif /* OCTET_EDITOR_H */
