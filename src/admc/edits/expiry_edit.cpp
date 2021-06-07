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

#include "edits/expiry_edit.h"

#include "edits/expiry_widget.h"
#include "adldap.h"
#include "globals.h"
#include "utils.h"

#include <QFormLayout>

ExpiryEdit::ExpiryEdit(QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent)
{
    edit_widget = new ExpiryWidget();

    connect(
        edit_widget, &ExpiryWidget::edited,
        [this]() {
            emit edited();
        });
}

void ExpiryEdit::load_internal(AdInterface &ad, const AdObject &object) {
    edit_widget->load(object);
}

void ExpiryEdit::set_read_only(const bool read_only) {
    edit_widget->set_read_only(read_only);
}

void ExpiryEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_ACCOUNT_EXPIRES, "") + ":";

    layout->addRow(label_text, edit_widget);
}

bool ExpiryEdit::apply(AdInterface &ad, const QString &dn) const {
    return edit_widget->apply(ad, dn);
}
