/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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
#include "core/settings.h"
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

        ui->specified_radio_button->setChecked(true);
        ui->specified_wget->setDisabled(false);
    }
    else {
        ui->all_radio_button->setChecked(true);
        ui->specified_wget->setDisabled(true);
    }

    ui->list->setStyleSheet("QListWidget { border: 1px solid palette(mid); }");

    enable_widget_on_selection(ui->remove_button, ui->list);

    settings_setup_dialog_geometry(SETTING_logon_computers_dialog_geometry, this);

    connect(
        ui->add_button, &QPushButton::clicked,
        this, &LogonComputersDialog::on_add_button);
    connect(
        ui->remove_button, &QPushButton::clicked,
        this, &LogonComputersDialog::on_remove_button);
    connect(
        ui->all_radio_button, &QRadioButton::clicked,
        this, &LogonComputersDialog::on_all_radio_button);
    connect(
        ui->specified_radio_button, &QRadioButton::clicked,
        this, &LogonComputersDialog::on_specified_radio_button);
}

LogonComputersDialog::~LogonComputersDialog() {
    delete ui;
}

QString LogonComputersDialog::get() const {
    if (ui->all_radio_button->isChecked()) {
        return QString();
    }

    QList<QString> value_list;
    for (int i = 0; i < ui->list->count(); i++) {
        QListWidgetItem *item = ui->list->item(i);
        value_list.append(item->text());
    }

    return value_list.join(",");
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

void LogonComputersDialog::on_all_radio_button() {
    ui->specified_wget->setDisabled(true);
}

void LogonComputersDialog::on_specified_radio_button() {
    ui->specified_wget->setEnabled(true);
}
