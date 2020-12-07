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

#include "tabs/telephones_tab.h"
#include "edits/string_other_edit.h"
#include "edits/string_large_edit.h"
#include "ad_defines.h"

#include <QFormLayout>

TelephonesTab::TelephonesTab() {
    new StringOtherEdit(ATTRIBUTE_HOME_PHONE, ATTRIBUTE_OTHER_HOME_PHONE, CLASS_USER, &edits, this);
    new StringOtherEdit(ATTRIBUTE_PAGER, ATTRIBUTE_OTHER_PAGER, CLASS_USER, &edits, this);
    new StringOtherEdit(ATTRIBUTE_MOBILE, ATTRIBUTE_OTHER_MOBILE, CLASS_USER, &edits, this);
    new StringOtherEdit(ATTRIBUTE_FAX_NUMBER, ATTRIBUTE_OTHER_FAX_NUMBER, CLASS_USER, &edits, this);
    new StringOtherEdit(ATTRIBUTE_IP_PHONE, ATTRIBUTE_OTHER_IP_PHONE, CLASS_USER, &edits, this);

    new StringLargeEdit(ATTRIBUTE_INFO, CLASS_USER, &edits, this);

    edits_connect_to_tab(edits, this);

    const auto layout = new QFormLayout();
    setLayout(layout);
    edits_add_to_layout(edits, layout);
}
