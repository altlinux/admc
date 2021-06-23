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

#include "edits/manager_widget.h"

#include "adldap.h"
#include "globals.h"
#include "properties_dialog.h"
#include "select_object_dialog.h"
#include "utils.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

ManagerWidget::ManagerWidget(const QString &manager_attribute_arg)
: QWidget() {
    manager_attribute = manager_attribute_arg;

    edit = new QLineEdit();
    edit->setReadOnly(true);

    change_button = new QPushButton(tr("Change"));
    properties_button = new QPushButton(PropertiesDialog::display_name());
    clear_button = new QPushButton(tr("Clear"));

    auto buttons_layout = new QHBoxLayout();
    buttons_layout->addWidget(change_button);
    buttons_layout->addWidget(properties_button);
    buttons_layout->addWidget(clear_button);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(edit);
    layout->addLayout(buttons_layout);

    connect(
        change_button, &QPushButton::clicked,
        this, &ManagerWidget::on_change);
    connect(
        properties_button, &QPushButton::clicked,
        this, &ManagerWidget::on_properties);
    connect(
        clear_button, &QPushButton::clicked,
        this, &ManagerWidget::on_clear);
}

void ManagerWidget::load(const AdObject &object) {
    const QString manager = object.get_string(manager_attribute);

    load_value(manager);
}

bool ManagerWidget::apply(AdInterface &ad, const QString &dn) const {
    const bool success = ad.attribute_replace_string(dn, manager_attribute, current_value);

    return success;
}

QString ManagerWidget::get_manager() const {
    return current_value;
}

void ManagerWidget::reset() {
    edit->setText(QString());
}

void ManagerWidget::on_change() {
    auto dialog = new SelectObjectDialog({CLASS_USER, CLASS_CONTACT}, SelectObjectDialogMultiSelection_No, edit);

    connect(
        dialog, &SelectObjectDialog::accepted,
        [this, dialog]() {
            const QList<QString> selected = dialog->get_selected();

            load_value(selected[0]);

            emit edited();
        });

    dialog->open();
}

void ManagerWidget::on_properties() {
    PropertiesDialog::open_for_target(current_value);
}

void ManagerWidget::on_clear() {
    load_value(QString());

    emit edited();
}

void ManagerWidget::load_value(const QString &value) {
    current_value = value;

    const QString rdn = dn_get_name(current_value);
    edit->setText(current_value);

    const bool have_manager = !current_value.isEmpty();
    properties_button->setEnabled(have_manager);
    clear_button->setEnabled(have_manager);
}
