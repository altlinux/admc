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

#include "tabs/managed_by_tab.h"

#include "adldap.h"
#include "edits/country_edit.h"
#include "edits/group_scope_edit.h"
#include "edits/group_type_edit.h"
#include "edits/manager_edit.h"
#include "edits/string_edit.h"
#include "edits/string_other_edit.h"
#include "utils.h"

#include <QFormLayout>

// NOTE: store manager's edits in separate list because they
// don't apply to the target of properties.

ManagedByTab::ManagedByTab() {
    manager_edit = new ManagerEdit(ATTRIBUTE_MANAGED_BY, &edits, this);

    const QList<QString> attributes = {
        ATTRIBUTE_OFFICE,
        ATTRIBUTE_STREET,
        ATTRIBUTE_CITY,
        ATTRIBUTE_STATE,
    };
    StringEdit::make_many(attributes, CLASS_USER, &manager_edits, this);

    new CountryEdit(&manager_edits, this);

    new StringOtherEdit(ATTRIBUTE_TELEPHONE_NUMBER, ATTRIBUTE_TELEPHONE_NUMBER_OTHER, CLASS_USER, &manager_edits, this);
    new StringOtherEdit(ATTRIBUTE_FAX_NUMBER, ATTRIBUTE_OTHER_FAX_NUMBER, CLASS_USER, &manager_edits, this);

    edits_set_read_only(manager_edits, true);

    auto edits_layout = new QFormLayout();

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addLayout(edits_layout);

    edits_add_to_layout(edits, edits_layout);
    edits_connect_to_tab(edits, this);

    edits_add_to_layout(manager_edits, edits_layout);
    edits_connect_to_tab(manager_edits, this);

    connect(
        manager_edit, &ManagerEdit::edited,
        this, &ManagedByTab::on_manager_edited);
}

void ManagedByTab::on_manager_edited() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    load_manager_edits(ad);
}

void ManagedByTab::load(AdInterface &ad, const AdObject &object) {
    manager_edit->load(ad, object);

    // NOTE: load AFTER loading manager! because manager
    // edits use current value of manager edit
    load_manager_edits(ad);
}

void ManagedByTab::load_manager_edits(AdInterface &ad) {
    const QString manager = manager_edit->get_manager();

    if (!manager.isEmpty()) {
        const AdObject manager_object = ad.search_object(manager);
        edits_load(manager_edits, ad, manager_object);
    } else {
        AdObject empty_object;
        edits_load(manager_edits, ad, empty_object);
    }
}
