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
#include "config.h"

#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QMessageBox>

StringMultiEditDialog::StringMultiEditDialog(const QString attribute, const QList<QByteArray> values)
: EditDialog()
{
    original_values = values;

    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(QString(tr("Edit %1 - %2")).arg(attribute, ADMC_APPLICATION_NAME));

    edit = new QLineEdit();
    if (ADCONFIG()->attribute_is_number(attribute)) {
        set_line_edit_to_numbers_only(edit);
    }

    ADCONFIG()->limit_edit(edit, attribute);

    add_button = new QPushButton(tr("Add"));

    list_widget = new QListWidget();

    remove_button = new QPushButton(tr("Remove"));

    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    auto reset_button = button_box->addButton(QDialogButtonBox::Reset);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(edit);
    top_layout->addWidget(add_button);
    top_layout->addWidget(list_widget);
    top_layout->addWidget(remove_button);
    top_layout->addWidget(button_box);

    if (ADCONFIG()->get_attribute_is_system_only(attribute)) {
        edit->setReadOnly(true);
        button_box->setEnabled(false);
        add_button->setEnabled(false);
    }

    connect(
        ok_button, &QPushButton::clicked,
        this, &QDialog::accept);
    connect(
        reset_button, &QPushButton::clicked,
        this, &StringMultiEditDialog::reset);
    connect(
        add_button, &QAbstractButton::clicked,
        this, &StringMultiEditDialog::on_add);
    connect(
        remove_button, &QAbstractButton::clicked,
        this, &StringMultiEditDialog::on_remove);
    connect(
        cancel_button, &QAbstractButton::clicked,
        this, &StringMultiEditDialog::reject);
    connect(
        edit, &QLineEdit::textChanged,
        this, &StringMultiEditDialog::on_edit_changed);
    on_edit_changed();
    connect(
        list_widget, &QListWidget::itemSelectionChanged,
        this, &StringMultiEditDialog::on_list_selected_changed);
    on_list_selected_changed();

    reset();
}

void StringMultiEditDialog::on_edit_changed() {
    const bool edit_has_text = !edit->text().isEmpty();
    add_button->setEnabled(edit_has_text);
}

void StringMultiEditDialog::on_list_selected_changed() {
    const bool any_selected = !list_widget->selectedItems().isEmpty();
    remove_button->setEnabled(any_selected);
}

void StringMultiEditDialog::on_add() {
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

void StringMultiEditDialog::on_remove() {
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

void StringMultiEditDialog::reset() {
    edit->clear();
    list_widget->clear();

    for (const QByteArray &value : original_values) {
        // TODO: use conversion f-n
        const QString value_string = QString::fromUtf8(value);

        list_widget->addItem(value_string);
    }
}
