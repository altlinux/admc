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

#include "attribute_multi_edits/expiry_multi_edit.h"

#include "adldap.h"
#include "attribute_edits/expiry_widget.h"
#include "globals.h"

#include <QCheckBox>
#include <QFormLayout>

ExpiryMultiEdit::ExpiryMultiEdit(ExpiryWidget *edit_widget_arg, QCheckBox *check, QList<AttributeMultiEdit *> *edit_list, QObject *parent)
: AttributeMultiEdit(check, edit_list, parent) {
    edit_widget = edit_widget_arg;

    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_ACCOUNT_EXPIRES, "") + ":";
    check->setText(label_text);
}

bool ExpiryMultiEdit::apply(AdInterface &ad, const QString &target) {
    return edit_widget->apply(ad, target);
}

void ExpiryMultiEdit::set_enabled(const bool enabled) {
    edit_widget->setEnabled(enabled);
}
