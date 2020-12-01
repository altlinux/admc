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
#include <QFont>
#include <QFontDatabase>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QListWidget>

// TODO: implement editing. Once single valued octet edit has it. Also display in different formats?

OctetMultiEditDialog::OctetMultiEditDialog(const QString attribute, const QList<QByteArray> values, QWidget *parent)
: EditDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Edit multi-valued octet string"));

    QLabel *attribute_label = make_attribute_label(attribute);

    add_button = new QPushButton(tr("Add"));

    list_widget = new QListWidget();
    for (const QByteArray &value : values) {
        const QString display_value = attribute_display_value(attribute, value);

        auto item = new QListWidgetItem(display_value);
        const QFont fixed_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        item->setFont(fixed_font);

        list_widget->addItem(item);
    }

    auto remove_button = new QPushButton(tr("Remove"));

    QDialogButtonBox *button_box = make_button_box(attribute);;

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(attribute_label);
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
        this, &OctetMultiEditDialog::on_add);
    connect(
        remove_button, &QAbstractButton::clicked,
        this, &OctetMultiEditDialog::on_remove);
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

QList<QByteArray> OctetMultiEditDialog::get_new_values() const {
    // TODO:
    return QList<QByteArray>();
}
