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

#include "multi_edits/expiry_multi_edit.h"

#include "adldap.h"
#include "edits/expiry_widget.h"
#include "globals.h"

#include <QFormLayout>
#include <QCheckBox>

ExpiryMultiEdit::ExpiryMultiEdit(ExpiryWidget *edit_widget_arg, QCheckBox *check, QList<AttributeMultiEdit *> &edits_out, QObject *parent)
: AttributeMultiEdit(check, edits_out, parent) {
    edit_widget = edit_widget_arg;

    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_ACCOUNT_EXPIRES, "") + ":";
    apply_check->setText(label_text);

    connect(
        edit_widget, &ExpiryWidget::edited,
        this, &ExpiryMultiEdit::edited);

    set_enabled(false);
}

bool ExpiryMultiEdit::apply_internal(AdInterface &ad, const QString &target) {
    return edit_widget->apply(ad, target);
}

void ExpiryMultiEdit::set_enabled(const bool enabled) {
    if (!enabled) {
        edit_widget->load(AdObject());
    }

    edit_widget->setEnabled(enabled);
}
