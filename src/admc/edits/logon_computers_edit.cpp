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

#include "edits/logon_computers_edit.h"
#include "edits/logon_computers_edit_p.h"
#include "edits/ui_logon_computers_dialog.h"

#include "adldap.h"

LogonComputersEdit::LogonComputersEdit(QPushButton *button_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    button = button_arg;
    
    dialog = new LogonComputersDialog(button);

    connect(
        button, &QPushButton::clicked,
        dialog, &QDialog::open);
    connect(
        dialog, &LogonComputersDialog::accepted,
        [this]() {
            emit edited();
        });
}

void LogonComputersEdit::load_internal(AdInterface &ad, const AdObject &object) {
    const 
    QString value = object.get_value(ATTRIBUTE_USER_WORKSTATIONS);
    dialog->load(value);
}

void LogonComputersEdit::set_read_only(const bool read_only) {
    button->setEnabled(read_only);
}

bool LogonComputersEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = dialog->get();
    const bool success = ad.attribute_replace_string(dn, ATTRIBUTE_USER_WORKSTATIONS, new_value);

    return success;
}

LogonComputersDialog::LogonComputersDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::LogonComputersDialog();
    ui->setupUi(this);

    connect(
        ui->add_button, &QPushButton::clicked,
        this, &LogonComputersDialog::on_add_button);
    connect(
        ui->remove_button, &QPushButton::clicked,
        this, &LogonComputersDialog::on_remove_button);
}

void LogonComputersDialog::load(const QString &value) {
    ui->list->clear();

    if (value.isEmpty()) {
        return;
    }

    const QList<QString> value_list = value.split(",");

    for (const QString &subvalue : value_list) {
        ui->list->addItem(subvalue);
    }
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
