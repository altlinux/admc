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

#include "attribute_dialogs/list_attribute_dialog.h"
#include "attribute_dialogs/ui_list_attribute_dialog.h"

#include "adldap.h"
#include "attribute_dialogs/bool_attribute_dialog.h"
#include "attribute_dialogs/datetime_attribute_dialog.h"
#include "attribute_dialogs/octet_attribute_dialog.h"
#include "attribute_dialogs/string_attribute_dialog.h"
#include "globals.h"
#include "utils.h"
#include "settings.h"

ListAttributeDialog::ListAttributeDialog(QWidget *parent)
: AttributeDialog(parent) {
    ui = new Ui::ListAttributeDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    AttributeDialog::set_attribute_label(ui->attribute_label);

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

void ListAttributeDialog::set_read_only(const bool read_only) {
    AttributeDialog::set_read_only(read_only);

    ui->add_button->setVisible(!read_only);
    ui->remove_button->setVisible(!read_only);
}

void ListAttributeDialog::on_add_button() {
    AttributeDialog *dialog = [this]() -> AttributeDialog * {
        const bool is_bool = (g_adconfig->get_attribute_type(get_attribute()) == AttributeType_Boolean);

        if (is_bool) {
            return new BoolAttributeDialog(this);
        } else {
            return new StringAttributeDialog(this);
        }
    }();

    dialog->set_attribute(get_attribute());
    dialog->set_value_list({});
    dialog->set_read_only(get_read_only());
    
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
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

void ListAttributeDialog::set_value_list(const QList<QByteArray> &values) {
    for (const QByteArray &value : values) {
        add_value(value);
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

void ListAttributeDialog::add_value(const QByteArray value) {
    const QString text = bytes_to_string(value);
    ui->list_widget->addItem(text);
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
