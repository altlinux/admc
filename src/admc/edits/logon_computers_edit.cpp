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

#include "adldap.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>

LogonComputersEdit::LogonComputersEdit(QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    button = new QPushButton(tr("Logon computers"));
    button->setObjectName("logon_computers_edit_button");
    
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

void LogonComputersEdit::add_to_layout(QFormLayout *layout) {
    auto edit_layout = new QHBoxLayout();
    edit_layout->addWidget(button);
    edit_layout->addStretch();

    layout->addRow(edit_layout);
}

bool LogonComputersEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = dialog->get();
    const bool success = ad.attribute_replace_string(dn, ATTRIBUTE_USER_WORKSTATIONS, new_value);

    return success;
}

LogonComputersDialog::LogonComputersDialog(QWidget *parent)
: QDialog(parent) {
    setWindowTitle("Logon computers");

    resize(400, 400);

    auto edit_label = new QLabel(tr("New value:"));

    edit = new QLineEdit();
    edit->setObjectName("edit");

    auto add_button = new QPushButton(tr("Add"));
    add_button->setObjectName("add_button");
    
    auto remove_button = new QPushButton(tr("Remove"));
    remove_button->setObjectName("remove_button");

    auto list_label = new QLabel(tr("Values:"));
    
    list = new QListWidget();
    list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    list->setObjectName("list");

    auto button_box = new QDialogButtonBox();
    button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);

    auto edit_layout = new QHBoxLayout();
    edit_layout->addWidget(edit_label);
    edit_layout->addWidget(edit);
    edit_layout->addWidget(add_button);

    auto list_layout = new QHBoxLayout();
    list_layout->addWidget(list);
    list_layout->addWidget(remove_button);
    list_layout->setAlignment(remove_button, Qt::AlignTop);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(edit_layout);
    layout->addWidget(list_label);
    layout->addLayout(list_layout);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
    connect(
        add_button, &QPushButton::clicked,
        this, &LogonComputersDialog::on_add_button);
    connect(
        remove_button, &QPushButton::clicked,
        this, &LogonComputersDialog::on_remove_button);
}

void LogonComputersDialog::load(const QString &value) {
    list->clear();

    if (value.isEmpty()) {
        return;
    }

    const QList<QString> value_list = value.split(",");

    for (const QString &subvalue : value_list) {
        list->addItem(subvalue);
    }
}

QString LogonComputersDialog::get() const {
    const QList<QString> value_list = [&]() {
        QList<QString> out;

        for (int i = 0; i < list->count(); i++) {
            QListWidgetItem *item = list->item(i);
            const QString value = item->text();
            out.append(value);
        }

        return out;
    }();

    const QString value_string = value_list.join(",");

    return value_string;
}

void LogonComputersDialog::on_add_button() {
    const QString value = edit->text();

    if (value.isEmpty()) {
        return;
    }

    list->addItem(value);

    edit->clear();
}

void LogonComputersDialog::on_remove_button() {
    const QList<QListWidgetItem *> selected = list->selectedItems();

    for (const QListWidgetItem *item : selected) {
        list->takeItem(list->row(item));
        delete item;
    }
}
