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

#include "editors/datetime_editor.h"
#include "editors/ui_datetime_editor.h"

#include "adldap.h"
#include "globals.h"

DateTimeEditor::DateTimeEditor(const QString attribute_arg, QWidget *parent)
: AttributeEditor(attribute_arg, parent) {
    ui = new Ui::DateTimeEditor();
    ui->setupUi(this);

    const bool system_only = g_adconfig->get_attribute_is_system_only(attribute);
    if (system_only) {
        ui->edit->setReadOnly(true);
    }

    init(ui->button_box, ui->attribute_label);
}

DateTimeEditor::~DateTimeEditor() {
    delete ui;
}

void DateTimeEditor::load(const QList<QByteArray> &values) {
    const QByteArray value = values.value(0, QByteArray());
    const QString value_string = QString(value);
    const QDateTime value_datetime = datetime_string_to_qdatetime(attribute, value_string, g_adconfig);
    ui->edit->setDateTime(value_datetime);
}

QList<QByteArray> DateTimeEditor::get_new_values() const {
    return QList<QByteArray>();
}
