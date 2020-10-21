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

#include "tabs/details_tab.h"
#include "edits/attribute_edit.h"

void DetailsTab::load(const AdObject &object) {
    edits_load(edits, object);
}

bool DetailsTab::changed() const {
    return edits_changed(edits);
}

bool DetailsTab::verify() const {
    return edits_verify(edits);
}

void DetailsTab::apply(const QString &target) const {
    edits_apply(edits, target);
}

void DetailsTab::on_edit_edited() {
    emit edited();
}
