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

#include "edits/manager_edit.h"
#include "utils.h"
#include "adldap.h"
#include "globals.h"
#include "properties_dialog.h"
#include "select_dialog.h"
#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>

ManagerEdit::ManagerEdit(const QString &manager_attribute_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent)
{
    manager_attribute = manager_attribute_arg;

    edit = new QLineEdit();

    change_button = new QPushButton(tr("Change"));
    properties_button = new QPushButton(PropertiesDialog::display_name());
    clear_button = new QPushButton(tr("Clear"));

    connect(
        change_button, &QPushButton::clicked,
        this, &ManagerEdit::on_change);
    connect(
        properties_button, &QPushButton::clicked,
        this, &ManagerEdit::on_properties);
    connect(
        clear_button, &QPushButton::clicked,
        this, &ManagerEdit::on_clear);
}

void ManagerEdit::load_internal(AdInterface &ad, const AdObject &object) {
    const QString manager = object.get_string(manager_attribute);
    
    load_value(manager);
}

void ManagerEdit::set_read_only(const bool read_only) {

}

void ManagerEdit::add_to_layout(QFormLayout *layout) {
    auto buttons_layout = new QHBoxLayout();
    buttons_layout->addWidget(change_button);
    buttons_layout->addWidget(properties_button);
    buttons_layout->addWidget(clear_button);

    auto sublayout = new QVBoxLayout();
    sublayout->addWidget(edit);
    sublayout->addLayout(buttons_layout);

    const QString label_text = g_adconfig->get_attribute_display_name(manager_attribute, CLASS_USER) + ":";
    layout->addRow(label_text, sublayout);
}

bool ManagerEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool success = ad.attribute_replace_string(dn, manager_attribute, current_value);

    return success;
}

QString ManagerEdit::get_manager() const {
    return current_value;
}

void ManagerEdit::on_change() {
    auto dialog = new SelectDialog({CLASS_USER, CLASS_CONTACT}, SelectDialogMultiSelection_No, edit);

    connect(
        dialog, &SelectDialog::accepted,
        [this, dialog]() {
            const QList<QString> selected = dialog->get_selected();

            load_value(selected[0]);

            emit edited();
        });

    dialog->open();
}

void ManagerEdit::on_properties() {
    PropertiesDialog::open_for_target(current_value);
}

void ManagerEdit::on_clear() {
    load_value(QString());

    emit edited();
}

void ManagerEdit::load_value(const QString &value) {
    current_value = value;

    const QString rdn = dn_get_name(current_value);
    edit->setText(current_value);

    const bool have_manager = !current_value.isEmpty();
    properties_button->setEnabled(have_manager);
    clear_button->setEnabled(have_manager);
}
