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

#include "attribute_edits/logon_computers_dialog.h"
#include "attribute_edits/ui_logon_computers_dialog.h"

#include "adldap.h"
#include "settings.h"
#include "utils.h"

#include <QPushButton>

LogonComputersDialog::LogonComputersDialog(const QString &value, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::LogonComputersDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    if (!value.isEmpty()) {
        const QList<QString> value_list = value.split(",");

        for (const QString &subvalue : value_list) {
            ui->list->addItem(subvalue);
        }
    }

    enable_widget_on_selection(ui->remove_button, ui->list);

    settings_setup_dialog_geometry(SETTING_logon_computers_dialog_geometry, this);

    connect(
        ui->add_button, &QPushButton::clicked,
        this, &LogonComputersDialog::on_add_button);
    connect(
        ui->remove_button, &QPushButton::clicked,
        this, &LogonComputersDialog::on_remove_button);
}

LogonComputersDialog::~LogonComputersDialog() {
    delete ui;
}

QString LogonComputersDialog::get() const {
    const QList<QString> value_list = [&]() {
        QList<QString> out;

        for (int i = 0; i < ui->list->count(); i++) {
            QListWidgetItem *item = ui->list->item(i);
            const QString value = item->text();
            out.append(value);
        }

        return out;
    }();

    const QString value_string = value_list.join(",");

    return value_string;
}

void LogonComputersDialog::on_add_button() {
    const QString value = ui->edit->text();

    if (value.isEmpty()) {
        return;
    }

    ui->list->addItem(value);

    ui->edit->clear();
}

void LogonComputersDialog::on_remove_button() {
    const QList<QListWidgetItem *> selected = ui->list->selectedItems();

    for (const QListWidgetItem *item : selected) {
        ui->list->takeItem(ui->list->row(item));
        delete item;
    }
}
