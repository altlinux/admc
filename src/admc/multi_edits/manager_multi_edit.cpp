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

#include "multi_edits/manager_multi_edit.h"

#include "adldap.h"
#include "edits/manager_widget.h"
#include "globals.h"

#include <QFormLayout>
#include <QCheckBox>

ManagerMultiEdit::ManagerMultiEdit(ManagerWidget *widget_arg, QCheckBox *check, QList<AttributeMultiEdit *> &edits_out, QObject *parent)
: AttributeMultiEdit(check, edits_out, parent) {
    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_MANAGER, CLASS_USER) + ":";
    apply_check->setText(label_text);

    widget = widget_arg;

    connect(
        widget, &ManagerWidget::edited,
        this, &AttributeMultiEdit::edited);

    set_enabled(false);
}

bool ManagerMultiEdit::apply_internal(AdInterface &ad, const QString &target) {
    return widget->apply(ad, target);
}

void ManagerMultiEdit::set_enabled(const bool enabled) {
    if (!enabled) {
        widget->reset();
    }

    widget->setEnabled(enabled);
}
