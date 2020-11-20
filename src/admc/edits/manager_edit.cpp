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
#include "ad_interface.h"
#include "ad_utils.h"
#include "ad_config.h"
#include "details_dialog.h"
#include "select_dialog.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>

ManagerEdit::ManagerEdit(QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent)
{
    edit = new QLineEdit();

    change_button = new QPushButton(tr("Change"));
    details_button = new QPushButton(tr("Details"));
    clear_button = new QPushButton(tr("Clear"));

    connect(
        change_button, &QPushButton::clicked,
        this, &ManagerEdit::on_change);
    connect(
        details_button, &QPushButton::clicked,
        this, &ManagerEdit::on_details);
    connect(
        clear_button, &QPushButton::clicked,
        this, &ManagerEdit::on_clear);
}

void ManagerEdit::load_internal(const AdObject &object) {
    const QString manager = object.get_string(ATTRIBUTE_MANAGER);
    
    load_value(manager);
}

void ManagerEdit::set_read_only(const bool read_only) {

}

void ManagerEdit::add_to_layout(QFormLayout *layout) {
    auto buttons_layout = new QHBoxLayout();
    buttons_layout->addWidget(change_button);
    buttons_layout->addWidget(details_button);
    buttons_layout->addWidget(clear_button);

    auto sublayout = new QVBoxLayout();
    sublayout->addWidget(edit);
    sublayout->addLayout(buttons_layout);

    const QString label_text = ADCONFIG()->get_attribute_display_name(ATTRIBUTE_MANAGER, CLASS_USER) + ":";
    layout->addRow(label_text, sublayout);
}

bool ManagerEdit::apply(const QString &dn) const {
    const bool success = AD()->attribute_replace_string(dn, ATTRIBUTE_MANAGER, current_value);

    return success;
}

void ManagerEdit::on_change() {
    const QList<QString> selected_objects = SelectDialog::open({CLASS_USER}, SelectDialogMultiSelection_No);

    if (selected_objects.size() > 0) {
        load_value(selected_objects[0]);

        emit edited();
    }
}

void ManagerEdit::on_details() {
    DetailsDialog::open_for_target(current_value);
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
    details_button->setEnabled(have_manager);
    clear_button->setEnabled(have_manager);
}
