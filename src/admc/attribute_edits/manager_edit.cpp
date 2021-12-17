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

#include "attribute_edits/manager_edit.h"

#include "adldap.h"
#include "attribute_edits/manager_widget.h"
#include "globals.h"
#include "utils.h"

ManagerEdit::ManagerEdit(ManagerWidget *widget_arg, const QString &manager_attribute_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    manager_attribute = manager_attribute_arg;

    widget = widget_arg;
    widget->set_attribute(manager_attribute_arg);

    connect(
        widget, &ManagerWidget::edited,
        this, &AttributeEdit::edited);
}

void ManagerEdit::load_internal(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    widget->load(object);
}

void ManagerEdit::set_read_only(const bool read_only) {
    widget->setEnabled(!read_only);
}

bool ManagerEdit::apply(AdInterface &ad, const QString &dn) {
    return widget->apply(ad, dn);
}

QString ManagerEdit::get_manager() const {
    return widget->get_manager();
}
