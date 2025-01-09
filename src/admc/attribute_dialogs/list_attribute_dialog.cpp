/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "attribute_dialogs/list_attribute_dialog.h"
#include "attribute_dialogs/ui_list_attribute_dialog.h"

#include "adldap.h"
#include "attribute_dialogs/attribute_dialog.h"
#include "attribute_dialogs/octet_attribute_dialog.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"

ListAttributeDialog::ListAttributeDialog(const QList<QByteArray> &value_list, const QString &attribute, const bool read_only, QWidget *parent)
: AttributeDialog(attribute, read_only, parent) {
    ui = new Ui::ListAttributeDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    // Default value indiciating no max length
    max_length = 0;

    AttributeDialog::load_attribute_label(ui->attribute_label);

    ui->add_button->setVisible(!read_only);
    ui->remove_button->setVisible(!read_only);

    for (const QByteArray &value : value_list) {
        add_value(value);
    }

    settings_setup_dialog_geometry(SETTING_list_attribute_dialog_geometry, this);

    connect(
        ui->add_button, &QAbstractButton::clicked,
        this, &ListAttributeDialog::on_add_button);
    connect(
        ui->remove_button, &QAbstractButton::clicked,
        this, &ListAttributeDialog::on_remove_button);
}

ListAttributeDialog::~ListAttributeDialog() {
    delete ui;
}

void ListAttributeDialog::accept() {
    const bool contains_empty_values = [&]() {
        const bool is_bool = (g_adconfig->get_attribute_type(get_attribute()) == AttributeType_Boolean);
        if (is_bool) {
            return false;
        }

        const QList<QByteArray> value_list = get_value_list();

        for (const QByteArray &value : value_list) {
            const QString value_string = QString(value);
            const bool value_is_all_spaces = (value.count(' ') == value.length());
            const bool value_is_empty = value.isEmpty() || value_is_all_spaces;

            if (value_is_empty) {
                return true;
            }
        }

        return false;
    }();

    if (contains_empty_values) {
        message_box_warning(this, tr("Error"), tr("One or more values are empty. Edit or remove them to proceed."));
    } else {
        QDialog::accept();
    }
}

void ListAttributeDialog::on_add_button() {
    const bool read_only = false;
    const bool single_valued = true;
    AttributeDialog *dialog = AttributeDialog::make(get_attribute(), QList<QByteArray>(), read_only, single_valued, this);

    if (dialog == nullptr) {
        return;
    }

    dialog->setWindowTitle(tr("Add Value"));
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog]() {
            const QList<QByteArray> new_values = dialog->get_value_list();

            if (!new_values.isEmpty()) {
                const QByteArray value = new_values[0];
                add_value(value);
            }
        });
}

void ListAttributeDialog::on_remove_button() {
    const QList<QListWidgetItem *> selected = ui->list_widget->selectedItems();

    for (const auto item : selected) {
        delete item;
    }
}

QList<QByteArray> ListAttributeDialog::get_value_list() const {
    QList<QByteArray> new_values;

    for (int i = 0; i < ui->list_widget->count(); i++) {
        const QListWidgetItem *item = ui->list_widget->item(i);
        const QString new_value_string = item->text();
        const QByteArray new_value = string_to_bytes(new_value_string);

        new_values.append(new_value);
    }

    return new_values;
}

void ListAttributeDialog::set_value_max_length(const int max_length_arg) {
    max_length = max_length_arg;
}

void ListAttributeDialog::add_value(const QByteArray value) {
    const QString text = bytes_to_string(value);
    const QList<QListWidgetItem *> find_results = ui->list_widget->findItems(text, Qt::MatchExactly);
    const bool value_already_exists = !find_results.isEmpty();

    if (!value_already_exists) {
        ui->list_widget->addItem(text);
    }
}

ListAttributeDialogType ListAttributeDialog::get_type() const {
    const AttributeType type = g_adconfig->get_attribute_type(get_attribute());

    switch (type) {
        case AttributeType_Octet: return ListAttributeDialogType_Octet;
        case AttributeType_Sid: return ListAttributeDialogType_Octet;

        case AttributeType_UTCTime: return ListAttributeDialogType_Datetime;
        case AttributeType_GeneralizedTime: return ListAttributeDialogType_Datetime;

        default: break;
    }

    return ListAttributeDialogType_String;
}

QString ListAttributeDialog::bytes_to_string(const QByteArray bytes) const {
    const ListAttributeDialogType editor_type = get_type();
    switch (editor_type) {
        case ListAttributeDialogType_String: return QString(bytes);
        case ListAttributeDialogType_Octet: return octet_bytes_to_string(bytes, OctetDisplayFormat_Hexadecimal);
        case ListAttributeDialogType_Datetime: return QString(bytes);
    }
    return QString();
}

QByteArray ListAttributeDialog::string_to_bytes(const QString string) const {
    const ListAttributeDialogType editor_type = get_type();

    switch (editor_type) {
        case ListAttributeDialogType_String: return string.toUtf8();
        case ListAttributeDialogType_Octet: return octet_string_to_bytes(string, OctetDisplayFormat_Hexadecimal);
        case ListAttributeDialogType_Datetime: return string.toUtf8();
    }

    return QByteArray();
}
