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

#include "editors/multi_editor.h"
#include "editors/ui_multi_editor.h"

#include "adldap.h"
#include "editors/bool_editor.h"
#include "editors/datetime_editor.h"
#include "editors/octet_editor.h"
#include "editors/string_editor.h"
#include "globals.h"
#include "utils.h"

MultiEditor::MultiEditor(QWidget *parent)
: AttributeEditor(parent) {
    ui = new Ui::MultiEditor();
    ui->setupUi(this);

    edited_item = nullptr;

    AttributeEditor::set_attribute_label(ui->attribute_label);

    // TODO: in read only case somehow disable this? maybe
    // disallow selection. but do want to allow copying of
    // values
    enable_widget_on_selection(ui->remove_button, ui->list_widget);

    add_string_editor = new StringEditor(this);
    add_bool_editor = new BoolEditor(this);

    edit_string_editor = new StringEditor(this);
    edit_octet_editor = new OctetEditor(this);
    edit_datetime_editor = new DateTimeEditor(this);

    connect(
        ui->add_button, &QAbstractButton::clicked,
        this, &MultiEditor::on_add_button);
    connect(
        ui->remove_button, &QAbstractButton::clicked,
        this, &MultiEditor::on_remove_button);
    connect(
        ui->list_widget, &QListWidget::itemDoubleClicked,
        this, &MultiEditor::on_item_double_clicked);
    connect(
        add_string_editor, &QDialog::accepted,
        this, &MultiEditor::on_add_string_editor_accepted);
    connect(
        add_bool_editor, &QDialog::accepted,
        this, &MultiEditor::on_add_bool_editor_accepted);
    connect(
        edit_string_editor, &QDialog::accepted,
        this, &MultiEditor::on_edit_string_editor_accepted);
    connect(
        edit_octet_editor, &QDialog::accepted,
        this, &MultiEditor::on_edit_octet_editor_accepted);
    connect(
        edit_datetime_editor, &QDialog::accepted,
        this, &MultiEditor::on_edit_datetime_editor_accepted);
}

MultiEditor::~MultiEditor() {
    delete ui;
}

void MultiEditor::set_attribute(const QString &attribute) {
    AttributeEditor::set_attribute(attribute);

    const QString title = [&]() {
        const AttributeType type = g_adconfig->get_attribute_type(attribute);

        const QString octet_title = tr("Edit Multi-Valued Octet");
        const QString datetime_title = tr("Edit Multi-Valued Datetime");

        switch (type) {
            case AttributeType_Integer: return tr("Edit Multi-Valued Integer");
            case AttributeType_LargeInteger: return tr("Edit Multi-Valued Large Integer");
            case AttributeType_Enumeration: return tr("Edit Multi-Valued Enumeration");
            case AttributeType_Boolean: return tr("Edit Multi-Valued Boolean");

            case AttributeType_Octet: return octet_title;
            case AttributeType_Sid: return octet_title;

            case AttributeType_UTCTime: return datetime_title;
            case AttributeType_GeneralizedTime: return datetime_title;

            default: break;
        };

        return tr("Edit Multi-Valued String");
    }();
    setWindowTitle(title);
}

void MultiEditor::set_read_only(const bool read_only) {
    ui->add_button->setEnabled(!read_only);
    ui->remove_button->setEnabled(!read_only);
}

void MultiEditor::on_add_button() {
    AttributeEditor *editor = [this]() -> AttributeEditor * {
        const bool is_bool = (g_adconfig->get_attribute_type(get_attribute()) == AttributeType_Boolean);

        if (is_bool) {
            return add_bool_editor;
        } else {
            return add_string_editor;
        }
    }();

    editor->set_attribute(get_attribute());
    editor->set_value_list({});
    editor->open();
}

void MultiEditor::on_remove_button() {
    const QList<QListWidgetItem *> selected = ui->list_widget->selectedItems();

    for (const auto item : selected) {
        delete item;
    }
}

void MultiEditor::set_value_list(const QList<QByteArray> &values) {
    for (const QByteArray &value : values) {
        add_value(value);
    }
}

QList<QByteArray> MultiEditor::get_value_list() const {
    QList<QByteArray> new_values;

    for (int i = 0; i < ui->list_widget->count(); i++) {
        const QListWidgetItem *item = ui->list_widget->item(i);
        const QString new_value_string = item->text();
        const QByteArray new_value = string_to_bytes(new_value_string);

        new_values.append(new_value);
    }

    return new_values;
}

void MultiEditor::on_item_double_clicked(QListWidgetItem *item) {
    const QString text = item->text();
    const QByteArray bytes = string_to_bytes(text);

    auto editor = [=]() -> AttributeEditor * {
        const MultiEditorType editor_type = get_editor_type();

        switch (editor_type) {
            case MultiEditorType_String: return edit_string_editor;
            case MultiEditorType_Octet: return edit_octet_editor;
            case MultiEditorType_Datetime: return edit_datetime_editor;
        }

        return nullptr;
    }();

    if (editor == nullptr) {
        return;
    }

    edited_item = item;

    editor->set_attribute(get_attribute());
    editor->set_value_list({bytes});
    editor->open();
}

void MultiEditor::add_value(const QByteArray value) {
    const QString text = bytes_to_string(value);
    ui->list_widget->addItem(text);
}

MultiEditorType MultiEditor::get_editor_type() const {
    const AttributeType type = g_adconfig->get_attribute_type(get_attribute());

    switch (type) {
        case AttributeType_Octet: return MultiEditorType_Octet;
        case AttributeType_Sid: return MultiEditorType_Octet;

        case AttributeType_UTCTime: return MultiEditorType_Datetime;
        case AttributeType_GeneralizedTime: return MultiEditorType_Datetime;

        default: break;
    }

    return MultiEditorType_String;
}

QString MultiEditor::bytes_to_string(const QByteArray bytes) const {
    const MultiEditorType editor_type = get_editor_type();
    switch (editor_type) {
        case MultiEditorType_String: return QString(bytes);
        case MultiEditorType_Octet: return octet_bytes_to_string(bytes, OctetDisplayFormat_Hexadecimal);
        case MultiEditorType_Datetime: return QString(bytes);
    }
    return QString();
}

QByteArray MultiEditor::string_to_bytes(const QString string) const {
    const MultiEditorType editor_type = get_editor_type();

    switch (editor_type) {
        case MultiEditorType_String: return string.toUtf8();
        case MultiEditorType_Octet: return octet_string_to_bytes(string, OctetDisplayFormat_Hexadecimal);
        case MultiEditorType_Datetime: return string.toUtf8();
    }

    return QByteArray();
}

void MultiEditor::on_add_string_editor_accepted() {
    add_values_from_editor(add_string_editor);
}

void MultiEditor::on_add_bool_editor_accepted() {
    add_values_from_editor(add_bool_editor);
}

void MultiEditor::add_values_from_editor(AttributeEditor *editor) {
    const QList<QByteArray> new_values = editor->get_value_list();

    if (!new_values.isEmpty()) {
        const QByteArray value = new_values[0];
        add_value(value);
    }
}

void MultiEditor::on_edit_string_editor_accepted() {
    edit_values_from_editor(edit_string_editor);
}

void MultiEditor::on_edit_octet_editor_accepted() {
    edit_values_from_editor(edit_octet_editor);
}

void MultiEditor::on_edit_datetime_editor_accepted() {
    edit_values_from_editor(edit_datetime_editor);
}

void MultiEditor::edit_values_from_editor(AttributeEditor *editor) {
    if (edited_item == nullptr) {
        return;
    }

    const QList<QByteArray> new_values = editor->get_value_list();

    if (!new_values.isEmpty()) {
        const QByteArray new_bytes = new_values[0];
        const QString new_text = bytes_to_string(new_bytes);

        edited_item->setText(new_text);
    }

    edited_item = nullptr;
}
