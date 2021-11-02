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

#ifndef BOOL_EDITOR_H
#define BOOL_EDITOR_H

#include "editors/attribute_editor.h"

namespace Ui {
class BoolEditor;
}

class BoolEditor final : public AttributeEditor {
    Q_OBJECT

public:
    Ui::BoolEditor *ui;

    BoolEditor(QWidget *parent);
    ~BoolEditor();

    void set_read_only(const bool read_only) override;
    void set_value_list(const QList<QByteArray> &values) override;
    QList<QByteArray> get_value_list() const override;
};

#endif /* BOOL_EDITOR_H */
