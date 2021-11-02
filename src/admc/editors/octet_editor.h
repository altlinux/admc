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

#ifndef OCTET_EDITOR_H
#define OCTET_EDITOR_H

#include "editors/attribute_editor.h"

enum OctetDisplayFormat {
    OctetDisplayFormat_Hexadecimal = 0,
    OctetDisplayFormat_Binary,
    OctetDisplayFormat_Decimal,
    OctetDisplayFormat_Octal,
};

namespace Ui {
class OctetEditor;
}

class OctetEditor final : public AttributeEditor {
    Q_OBJECT

public:
    Ui::OctetEditor *ui;

    OctetEditor(QWidget *parent);
    ~OctetEditor();

    void set_read_only(const bool read_only) override;
    void set_value_list(const QList<QByteArray> &values) override;
    QList<QByteArray> get_value_list() const override;
    void accept() override;

private:
    OctetDisplayFormat prev_format;

    bool check_input(const OctetDisplayFormat format);
    void on_format_combo();
};

QString octet_bytes_to_string(const QByteArray bytes, const OctetDisplayFormat format);
QByteArray octet_string_to_bytes(const QString string, const OctetDisplayFormat format);

#endif /* OCTET_EDITOR_H */
