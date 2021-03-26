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

#include "tabs/properties_tab.h"
#include "edits/attribute_edit.h"

#include "adldap.h"

void PropertiesTab::load(AdInterface &ad, const AdObject &object) {
    edits_load(edits, ad, object);
}

bool PropertiesTab::verify(AdInterface &ad, const QString &target) const {
    return edits_verify(ad, edits, target);
}

bool PropertiesTab::apply(AdInterface &ad, const QString &target) {
    return edits_apply(ad, edits, target);
}

void PropertiesTab::on_edit_edited() {
    emit edited();
}
