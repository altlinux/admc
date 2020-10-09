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

#include "attributes_tab_dialog_string_multi.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "edits/attribute_edit.h"
#include "edits/string_edit.h"
#include "status.h"
#include "utils.h"

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QMessageBox>

AttributesTabDialogStringMulti::AttributesTabDialogStringMulti(const QString attribute, const QList<QByteArray> values)
: AttributesTabDialog()
{
    original_values = values;

    setAttribute(Qt::WA_DeleteOnClose);
    resize(300, 300);

    const auto title_text = QString(tr("Edit %1")).arg(attribute);
    const auto title_label = new QLabel(title_text);

    edit = new QLineEdit();

    add_button = new QPushButton(tr("Add"));

    list_widget = new QListWidget();

    remove_button = new QPushButton(tr("Remove"));
    auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(title_label);
    top_layout->addWidget(edit);
    top_layout->addWidget(add_button);
    top_layout->addWidget(list_widget);
    top_layout->addWidget(remove_button);
    top_layout->addWidget(button_box);

    reset();

    connect(
        button_box->button(QDialogButtonBox::Ok), &QPushButton::clicked,
        this, &QDialog::accept);
    connect(
        button_box->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
        this, &AttributesTabDialogStringMulti::on_cancel);
    connect(
        add_button, &QAbstractButton::clicked,
        this, &AttributesTabDialogStringMulti::on_add);
    connect(
        remove_button, &QAbstractButton::clicked,
        this, &AttributesTabDialogStringMulti::on_remove);
    connect(
        edit, &QLineEdit::textChanged,
        this, &AttributesTabDialogStringMulti::on_edit_changed);
    on_edit_changed();
    connect(
        list_widget, &QListWidget::itemSelectionChanged,
        this, &AttributesTabDialogStringMulti::on_list_selected_changed);
    on_list_selected_changed();
}

void AttributesTabDialogStringMulti::on_edit_changed() {
    const bool edit_has_text = !edit->text().isEmpty();
    add_button->setEnabled(edit_has_text);
}

void AttributesTabDialogStringMulti::on_list_selected_changed() {
    const bool any_selected = !list_widget->selectedItems().isEmpty();
    remove_button->setEnabled(any_selected);
}

void AttributesTabDialogStringMulti::on_cancel() {
    reset();
}

void AttributesTabDialogStringMulti::on_add() {
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

void AttributesTabDialogStringMulti::on_remove() {
    const QList<QListWidgetItem *> selected = list_widget->selectedItems();

    for (const auto item : selected) {
        list_widget->removeItemWidget(item);
        delete item;
    }
}

QList<QByteArray> AttributesTabDialogStringMulti::get_new_values() const {
    QList<QByteArray> new_values;

    for (int i = 0; i < list_widget->count(); i++) {
        const QListWidgetItem *item = list_widget->item(i);
        const QString new_value_string = item->text();
        const QByteArray new_value = new_value_string.toUtf8();

        new_values.append(new_value);
    }

    return new_values;
}

void AttributesTabDialogStringMulti::reset() {
    edit->clear();
    list_widget->clear();

    for (const QByteArray &value : original_values) {
        // TODO: use conversion f-n
        const QString value_string = QString::fromUtf8(value);

        list_widget->addItem(value_string);
    }
}
