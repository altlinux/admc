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

#include "edit_dialogs/string_multi_edit_dialog.h"
#include "ad_config.h"
#include "utils.h"

#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QLabel>

StringMultiEditDialog::StringMultiEditDialog(const QString attribute, const QList<QByteArray> values, QWidget *parent)
: EditDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    const QString title =
    [attribute]() {
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

    edit = new QLineEdit();
    if (ADCONFIG()->attribute_is_number(attribute)) {
        set_line_edit_to_numbers_only(edit);
    }

    ADCONFIG()->limit_edit(edit, attribute);

    add_button = new QPushButton(tr("Add"));

    list_widget = new QListWidget();

    for (const QByteArray &value : values) {
        const QString value_string = QString(value);
        list_widget->addItem(value_string);
    }

    auto remove_button = new QPushButton(tr("Remove"));

    QDialogButtonBox *button_box = make_button_box(attribute);;

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(attribute_label);
    top_layout->addWidget(edit);
    top_layout->addWidget(add_button);
    top_layout->addWidget(list_widget);
    top_layout->addWidget(remove_button);
    top_layout->addWidget(button_box);

    const bool read_only = ADCONFIG()->get_attribute_is_system_only(attribute);
    if (read_only) {
        edit->setReadOnly(true);
        add_button->setEnabled(false);
        remove_button->setEnabled(false);
    } else {
        enable_widget_on_selection(remove_button, list_widget);
    }

    connect(
        add_button, &QAbstractButton::clicked,
        this, &StringMultiEditDialog::add);
    connect(
        remove_button, &QAbstractButton::clicked,
        this, &StringMultiEditDialog::remove);
    connect(
        edit, &QLineEdit::textChanged,
        this, &StringMultiEditDialog::enable_add_button_if_edit_not_empty);
    enable_add_button_if_edit_not_empty();
}

void StringMultiEditDialog::enable_add_button_if_edit_not_empty() {
    const bool edit_has_text = !edit->text().isEmpty();
    add_button->setEnabled(edit_has_text);
}

void StringMultiEditDialog::add() {
    const QString new_value = edit->text();

    const bool duplicate =
    [this, new_value]() {
        const QList<QListWidgetItem *> items = list_widget->findItems(new_value, Qt::MatchExactly);

        return !items.isEmpty();
    }();

    if (duplicate) {
        QMessageBox::warning(this, tr("Error"), tr("Value already exists"));
    } else {
        list_widget->addItem(new_value);
        edit->clear();
    }
}

void StringMultiEditDialog::remove() {
    const QList<QListWidgetItem *> selected = list_widget->selectedItems();

    for (const auto item : selected) {
        list_widget->removeItemWidget(item);
        delete item;
    }
}

QList<QByteArray> StringMultiEditDialog::get_new_values() const {
    QList<QByteArray> new_values;

    for (int i = 0; i < list_widget->count(); i++) {
        const QListWidgetItem *item = list_widget->item(i);
        const QString new_value_string = item->text();
        const QByteArray new_value = new_value_string.toUtf8();

        new_values.append(new_value);
    }

    return new_values;
}
