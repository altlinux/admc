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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ObjectTab::ObjectTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    title = tr("Object");

    // TODO: don't know why, but if i just have hbox as top layout, the widgets are misaligned
    const auto top_layout = new QVBoxLayout(this);

    const auto attributes_layout = new QHBoxLayout();
    top_layout->insertLayout(-1, attributes_layout);

    const auto label_layout = new QVBoxLayout();
    const auto edit_layout = new QVBoxLayout();
    attributes_layout->insertLayout(-1, label_layout);
    attributes_layout->insertLayout(-1, edit_layout);

    auto add_edit =
    [this, label_layout, edit_layout](const QString &attribute, const QString &label_text) {
        add_attribute_edit(attribute, label_text, label_layout, edit_layout, AttributeEditType_ReadOnly);
    };

    // TODO: canonical name in ADUC replaces "CN=" with "/" making it look like a directory path
    add_edit(ATTRIBUTE_DISTINGUISHED_NAME, tr("Canonical name:"));
    add_edit(ATTRIBUTE_OBJECT_CLASS, tr("Object class:"));
    add_edit(ATTRIBUTE_WHEN_CREATED, tr("Created:"));
    add_edit(ATTRIBUTE_WHEN_CHANGED, tr("Changed:"));
    add_edit(ATTRIBUTE_USN_CREATED, tr("USN created:"));
    add_edit(ATTRIBUTE_USN_CHANGED, tr("USN changed:"));
}

void ObjectTab::apply() {

}

void ObjectTab::cancel() {

}

void ObjectTab::reload_internal() {

}

bool ObjectTab::accepts_target() const {
    return AdInterface::instance()->has_attributes(target());
}
