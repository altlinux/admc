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

#include "editors/string_multi_editor.h"
#include "editors/string_editor.h"
#include "ad_config.h"
#include "utils.h"

#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QLabel>

StringMultiEditor::StringMultiEditor(const QString attribute_arg, const QList<QByteArray> values, QWidget *parent)
: AttributeEditor(parent)
{
    attribute = attribute_arg;

    const QString title =
    [this]() {
        const AttributeType type = ADCONFIG()->get_attribute_type(attribute);

        switch (type) {
            case AttributeType_Integer: return tr("Edit  multi-valued integer");
            case AttributeType_LargeInteger: return tr("Edit  multi-valued large integer");
            case AttributeType_Enumeration: return tr("Edit  multi-valued enumeration");
            default: break;
        };

        return tr("Edit multi-valued string");
    }();
    setWindowTitle(title);

    QLabel *attribute_label = make_attribute_label(attribute);

    auto add_button = new QPushButton(tr("Add"));

    list_widget = new QListWidget();

    for (const QByteArray &value : values) {
        add_value(value);
    }

    auto remove_button = new QPushButton(tr("Remove"));

    QDialogButtonBox *button_box = make_button_box(attribute);;

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(attribute_label);
    top_layout->addWidget(add_button);
    top_layout->addWidget(list_widget);
    top_layout->addWidget(remove_button);
    top_layout->addWidget(button_box);

    const bool read_only = ADCONFIG()->get_attribute_is_system_only(attribute);
    if (read_only) {
        add_button->setEnabled(false);
        remove_button->setEnabled(false);
    } else {
        enable_widget_on_selection(remove_button, list_widget);
    }

    connect(
        add_button, &QAbstractButton::clicked,
        this, &StringMultiEditor::add);
    connect(
        remove_button, &QAbstractButton::clicked,
        this, &StringMultiEditor::remove);
    connect(
        list_widget, &QListWidget::itemDoubleClicked,
        this, &StringMultiEditor::edit_item);
}

void StringMultiEditor::add() {
    auto editor = new StringEditor(attribute, QList<QByteArray>(), this);

    connect(
        editor, &QDialog::accepted,
        [this, editor]() {
            const QList<QByteArray> new_values = editor->get_new_values();

            if (!new_values.isEmpty()) {
                const QByteArray value = new_values[0];
                add_value(value);
            }
        });

    editor->open();
}

void StringMultiEditor::remove() {
    const QList<QListWidgetItem *> selected = list_widget->selectedItems();

    for (const auto item : selected) {
        delete item;
    }
}

QList<QByteArray> StringMultiEditor::get_new_values() const {
    QList<QByteArray> new_values;

    for (int i = 0; i < list_widget->count(); i++) {
        const QListWidgetItem *item = list_widget->item(i);
        const QString new_value_string = item->text();
        const QByteArray new_value = new_value_string.toUtf8();

        new_values.append(new_value);
    }

    return new_values;
}

void StringMultiEditor::edit_item(QListWidgetItem *item) {
    const QString text = item->text();
    const QByteArray bytes = text.toUtf8();

    auto editor = new StringEditor(attribute, {bytes}, this);

    connect(
        editor, &QDialog::accepted,
        [this, editor, item]() {
            const QList<QByteArray> new_values = editor->get_new_values();

            if (!new_values.isEmpty()) {
                const QByteArray new_bytes = new_values[0];
                const QString new_text = QString(new_bytes);

                item->setText(new_text);
            }
        });

    editor->open();
}

void StringMultiEditor::add_value(const QByteArray value) {
    const QString text = QString(value);
    list_widget->addItem(text);
}
