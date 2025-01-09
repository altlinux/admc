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

#include "attribute_dialogs/datetime_attribute_dialog.h"
#include "attribute_dialogs/ui_datetime_attribute_dialog.h"

#include "adldap.h"
#include "globals.h"
#include "settings.h"

DatetimeAttributeDialog::DatetimeAttributeDialog(const QList<QByteArray> &value_list, const QString &attribute, const bool read_only, QWidget *parent)
: AttributeDialog(attribute, read_only, parent) {
    ui = new Ui::DatetimeAttributeDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    AttributeDialog::load_attribute_label(ui->attribute_label);

    ui->edit->setReadOnly(read_only);

    const QByteArray value = value_list.value(0, QByteArray());
    const QString value_string = QString(value);
    const QDateTime value_datetime = datetime_string_to_qdatetime(get_attribute(), value_string, g_adconfig);
    ui->edit->setDateTime(value_datetime);

    settings_setup_dialog_geometry(SETTING_datetime_attribute_dialog_geometry, this);
}

DatetimeAttributeDialog::~DatetimeAttributeDialog() {
    delete ui;
}

QList<QByteArray> DatetimeAttributeDialog::get_value_list() const {
    const QDateTime value_datetime = ui->edit->dateTime();
    const QString value_string = datetime_qdatetime_to_string(get_attribute(), value_datetime, g_adconfig);
    const QByteArray value = value_string.toUtf8();
    const QList<QByteArray> value_list = {value};

    return value_list;
}
