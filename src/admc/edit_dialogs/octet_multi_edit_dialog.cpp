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

#include "edit_dialogs/octet_multi_edit_dialog.h"

#include "edit_dialogs/octet_edit_dialog.h"
#include "ad_config.h"
#include "utils.h"
#include "attribute_display.h"

#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QFont>
#include <QFontDatabase>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QListWidget>

// TODO: implement editing. Once single valued octet edit has it. Also display in different formats?

OctetMultiEditDialog::OctetMultiEditDialog(const QString attribute_arg, const QList<QByteArray> values, QWidget *parent)
: EditDialog(parent)
{
    attribute = attribute_arg;
    original_values = values;
    
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Edit multi-valued octet string"));

    add_button = new QPushButton(tr("Add"));

    list_widget = new QListWidget();
    
    remove_button = new QPushButton(tr("Remove"));

    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    auto reset_button = button_box->addButton(QDialogButtonBox::Reset);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    add_attribute_label(top_layout, attribute);
    top_layout->addWidget(add_button);
    top_layout->addWidget(list_widget);
    top_layout->addWidget(remove_button);
    top_layout->addWidget(button_box);

    if (ADCONFIG()->get_attribute_is_system_only(attribute)) {
        add_button->setEnabled(false);
        remove_button->setEnabled(false);
        button_box->setEnabled(false);
    }

    connect(
        ok_button, &QPushButton::clicked,
        this, &QDialog::accept);
    connect(
        reset_button, &QPushButton::clicked,
        this, &OctetMultiEditDialog::reset);
    connect(
        add_button, &QAbstractButton::clicked,
        this, &OctetMultiEditDialog::on_add);
    connect(
        remove_button, &QAbstractButton::clicked,
        this, &OctetMultiEditDialog::on_remove);
    connect(
        cancel_button, &QAbstractButton::clicked,
        this, &OctetMultiEditDialog::reject);
    connect(
        list_widget, &QListWidget::itemSelectionChanged,
        this, &OctetMultiEditDialog::on_list_selected_changed);
    on_list_selected_changed();

    reset();
}

void OctetMultiEditDialog::on_list_selected_changed() {
    const bool any_selected = !list_widget->selectedItems().isEmpty();
    remove_button->setEnabled(any_selected);
}

void OctetMultiEditDialog::on_add() {
    // TODO: open octet edit dialog and load new value from it into list widget
}

void OctetMultiEditDialog::on_remove() {
    const QList<QListWidgetItem *> selected = list_widget->selectedItems();

    for (const auto item : selected) {
        list_widget->removeItemWidget(item);
        delete item;
    }
}

void OctetMultiEditDialog::reset() {
    list_widget->clear();

    for (const QByteArray &value : original_values) {
        const QString display_value = attribute_display_value(attribute, value);

        auto item = new QListWidgetItem(display_value);
        const QFont fixed_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        item->setFont(fixed_font);

        list_widget->addItem(item);
    }
}

QList<QByteArray> OctetMultiEditDialog::get_new_values() const {
    // TODO:

    return original_values;
}
