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

#include "object_tab.h"
#include "ad_interface.h"
#include "attribute_edit.h"

#include <QVBoxLayout>
#include <QGridLayout>

// TODO: canonical name in ADUC replaces "CN=" with "/" making it look like a directory path

ObjectTab::ObjectTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    title = tr("Object");

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);

    const auto edits_layout = new QGridLayout();
    top_layout->addLayout(edits_layout);

    const QList<QString> attributes = {
        ATTRIBUTE_DISTINGUISHED_NAME,
        ATTRIBUTE_OBJECT_CLASS,
        ATTRIBUTE_WHEN_CREATED,
        ATTRIBUTE_WHEN_CHANGED,
        ATTRIBUTE_USN_CREATED,
        ATTRIBUTE_USN_CHANGED
    };
    for (auto attribute : attributes) {
        AttributeEdit *edit;
        if (attribute_is_datetime(attribute)) {
            edit = new DateTimeEdit(attribute, EditReadOnly_Yes);
        } else {
            edit = new StringEdit(attribute, EditReadOnly_Yes);
        }
        edit->add_to_layout(edits_layout);

        edits.append(edit);
    }

    connect_edits_to_details(edits, details);
}

void ObjectTab::apply() {

}

void ObjectTab::reload_internal() {
    for (auto edit : edits) {
        edit->load(target());
    }
}

bool ObjectTab::accepts_target() const {
    return AdInterface::instance()->has_attributes(target());
}
