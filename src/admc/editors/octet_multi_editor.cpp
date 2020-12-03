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

#include "editors/octet_multi_editor.h"

#include "editors/octet_editor.h"
#include "ad_config.h"
#include "utils.h"
#include "attribute_display.h"

#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QFont>
#include <QFontDatabase>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QListWidget>
#include <QComboBox>

OctetMultiEditor::OctetMultiEditor(const QString attribute_arg, const QList<QByteArray> values, QWidget *parent)
: AttributeEditor(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Edit multi-valued octet string"));

    attribute = attribute_arg;

    QLabel *attribute_label = make_attribute_label(attribute);

    format_combo = make_format_combo();

    add_button = new QPushButton(tr("Add"));

    list_widget = new QListWidget();
    for (const QByteArray &value : values) {
        add_value(value);
    }

    auto remove_button = new QPushButton(tr("Remove"));

    QDialogButtonBox *button_box = make_button_box(attribute);;

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(attribute_label);
    layout->addWidget(format_combo);
    layout->addWidget(add_button);
    layout->addWidget(list_widget);
    layout->addWidget(remove_button);
    layout->addWidget(button_box);

    const bool system_only = ADCONFIG()->get_attribute_is_system_only(attribute);
    if (system_only) {
        add_button->setEnabled(false);
        remove_button->setEnabled(false);
    } else {
        enable_widget_on_selection(remove_button, list_widget);
    }

    connect(
        add_button, &QAbstractButton::clicked,
        this, &OctetMultiEditor::on_add);
    connect(
        remove_button, &QAbstractButton::clicked,
        this, &OctetMultiEditor::on_remove);
    connect(
        list_widget, &QListWidget::itemDoubleClicked,
        this, &OctetMultiEditor::edit_item);
    connect(
        format_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &OctetMultiEditor::on_format_combo);
}

QList<QByteArray> OctetMultiEditor::get_new_values() const {
    QList<QByteArray> out;

    for (int i = 0; i < list_widget->count(); i++) {
        QListWidgetItem *item = list_widget->item(i);
        const QString text = item->text();
        const QByteArray bytes = string_to_bytes(text, current_format(format_combo));

        out.append(bytes);
    }

    return out;
}

void OctetMultiEditor::on_format_combo() {
    static OctetDisplayFormat prev_format = OctetDisplayFormat_Hexadecimal;

    // NOTE: don't need to check input before changing
    // format because values are checked before they are
    // added

    // Change format of all items
    for (int i = 0; i < list_widget->count(); i++) {
        QListWidgetItem *item = list_widget->item(i);
        const QString old_text = item->text();
        const QString new_text = string_change_format(old_text, prev_format, current_format(format_combo));

        item->setText(new_text);
    }

    prev_format = current_format(format_combo);
}

void OctetMultiEditor::edit_item(QListWidgetItem *item) {
    const QString text = item->text();
    const QByteArray bytes = string_to_bytes(text, current_format(format_combo));

    auto editor = new OctetEditor(attribute, {bytes}, this);

    connect(
        editor, &QDialog::accepted,
        [this, editor, item]() {
            const QList<QByteArray> new_values = editor->get_new_values();

            if (!new_values.isEmpty()) {
                const QByteArray new_bytes = new_values[0];
                const QString new_text = bytes_to_string(new_bytes, current_format(format_combo));

                item->setText(new_text);
            }
        });

    editor->open();
}

void OctetMultiEditor::on_add() {
    auto editor = new OctetEditor(attribute, QList<QByteArray>(), this);

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

void OctetMultiEditor::on_remove() {
    const QList<QListWidgetItem *> selected = list_widget->selectedItems();

    for (const auto item : selected) {
        delete item;
    }
}

void OctetMultiEditor::add_value(const QByteArray value) {
    const QString text = bytes_to_string(value, current_format(format_combo));

    auto item = new QListWidgetItem(text);
    const QFont fixed_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    item->setFont(fixed_font);

    list_widget->addItem(item);
}
