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

#include "tabs/object_tab.h"
#include "edits/string_edit.h"
#include "edits/datetime_edit.h"
#include "ad_defines.h"

#include <QFormLayout>

// TODO: canonical name in ADUC replaces "CN=" with "/" making it look like a directory path

ObjectTab::ObjectTab() {   
    new StringEdit(ATTRIBUTE_DISTINGUISHED_NAME, "", &edits, this);
    new StringEdit(ATTRIBUTE_OBJECT_CLASS, "", &edits, this);
    
    new DateTimeEdit(ATTRIBUTE_WHEN_CREATED, &edits, this);
    new DateTimeEdit(ATTRIBUTE_WHEN_CHANGED, &edits, this);

    // TODO: use int edit for this when/if it gets added, though these attributes aren't supposed to be editable anyway?
    new StringEdit(ATTRIBUTE_USN_CREATED, "", &edits, this);
    new StringEdit(ATTRIBUTE_USN_CHANGED, "", &edits, this);

    for (auto edit : edits) {
        edit->set_read_only(true);
    }

    edits_connect_to_tab(edits, this);

    const auto layout = new QFormLayout();
    setLayout(layout);
    edits_add_to_layout(edits, layout);
}
