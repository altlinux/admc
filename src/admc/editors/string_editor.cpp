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

#include "editors/string_editor.h"
#include "editors/ui_string_editor.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"
#include "settings.h"

StringEditor::StringEditor(QWidget *parent)
: AttributeEditor(parent) {
    ui = new Ui::StringEditor();
    ui->setupUi(this);

    AttributeEditor::set_attribute_label(ui->attribute_label);

    settings_setup_dialog_geometry(SETTING_multi_editor_geometry, this);
}

StringEditor::~StringEditor() {
    delete ui;
}

void StringEditor::set_attribute(const QString &attribute) {
    AttributeEditor::set_attribute(attribute);

    const QString title = [&]() {
        const AttributeType type = g_adconfig->get_attribute_type(attribute);

        switch (type) {
            case AttributeType_Integer: return tr("Edit Integer");
            case AttributeType_LargeInteger: return tr("Edit Large Integer");
            case AttributeType_Enumeration: return tr("Edit Enumeration");
            default: break;
        };

        return tr("Edit String");
    }();
    setWindowTitle(title);

    // Configure line edit based on attribute type
    if (g_adconfig->get_attribute_is_number(attribute)) {
        set_line_edit_to_numbers_only(ui->edit);
    }

    limit_edit(ui->edit, attribute);
}

void StringEditor::set_read_only(const bool read_only) {
    ui->edit->setReadOnly(read_only);
}

void StringEditor::set_value_list(const QList<QByteArray> &values) {
    const QByteArray value = values.value(0, QByteArray());
    const QString value_string = QString(value);
    ui->edit->setText(value_string);
}

QList<QByteArray> StringEditor::get_value_list() const {
    const QString new_value_string = ui->edit->text();

    if (new_value_string.isEmpty()) {
        return {};
    } else {
        const QByteArray new_value = new_value_string.toUtf8();
        return {new_value};
    }
}
