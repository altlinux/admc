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

#include "multi_edits/attribute_multi_edit.h"

#include "multi_tabs/properties_multi_tab.h"

#include <QDebug>

AttributeMultiEdit::AttributeMultiEdit(QList<AttributeMultiEdit *> *edits_out, QObject *parent)
: QObject(parent)
{
    if (edits_out != nullptr) {
        if (edits_out->contains(this)) {
            qDebug() << "ERROR: attribute edit added twice to list!";
        } else {
            edits_out->append(this);
        }
    }
}

void multi_edits_connect_to_tab(QList<AttributeMultiEdit *> edits, PropertiesMultiTab *tab) {
    for (auto edit : edits) {
        QObject::connect(
            edit, &AttributeMultiEdit::edited,
            tab, &PropertiesMultiTab::on_edit_edited);
    }
}

void multi_edits_add_to_layout(QList<AttributeMultiEdit *> edits, QFormLayout *layout) {
    for (auto edit : edits) {
        edit->add_to_layout(layout);
    }
}
