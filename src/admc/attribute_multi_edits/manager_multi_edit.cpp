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

#include "attribute_multi_edits/manager_multi_edit.h"

#include "adldap.h"
#include "attribute_edits/manager_widget.h"
#include "globals.h"

#include <QCheckBox>
#include <QFormLayout>

ManagerMultiEdit::ManagerMultiEdit(ManagerWidget *widget_arg, QCheckBox *check, QList<AttributeMultiEdit *> *edit_list, QObject *parent)
: AttributeMultiEdit(check, edit_list, parent) {
    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_MANAGER, CLASS_USER) + ":";
    check->setText(label_text);

    widget = widget_arg;
}

bool ManagerMultiEdit::apply(AdInterface &ad, const QString &target) {
    return widget->apply(ad, target);
}

void ManagerMultiEdit::set_enabled(const bool enabled) {
    widget->setEnabled(enabled);
}
