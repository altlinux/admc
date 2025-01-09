/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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
#include "tabs/ui_managed_by_tab.h"

#include "adldap.h"
#include "attribute_edits/country_edit.h"
#include "attribute_edits/group_scope_edit.h"
#include "attribute_edits/group_type_edit.h"
#include "attribute_edits/manager_edit.h"
#include "attribute_edits/string_edit.h"
#include "attribute_edits/string_other_edit.h"
#include "utils.h"

// NOTE: store manager's edits in separate list because they
// don't apply to the target of properties.

ManagedByTab::ManagedByTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::ManagedByTab();
    ui->setupUi(this);

    auto tab_edit = new ManagedByTabEdit(ui, this);

    edit_list->append({
        tab_edit,
    });
}

ManagedByTabEdit::ManagedByTabEdit(Ui::ManagedByTab *ui_arg, QObject *parent)
: AttributeEdit(parent) {
    ui = ui_arg;

    manager_edit = new ManagerEdit(ui->manager_widget, ATTRIBUTE_MANAGED_BY, this);

    auto office_edit = new StringEdit(ui->office_edit, ATTRIBUTE_OFFICE, this);
    auto street_edit = new StringEdit(ui->street_edit, ATTRIBUTE_STREET, this);
    auto city_edit = new StringEdit(ui->city_edit, ATTRIBUTE_CITY, this);
    auto state_edit = new StringEdit(ui->state_edit, ATTRIBUTE_STATE, this);

    auto country_edit = new CountryEdit(ui->country_combo, this);

    auto telephone_edit = new StringOtherEdit(ui->telephone_edit, ui->telephone_button, ATTRIBUTE_TELEPHONE_NUMBER, ATTRIBUTE_TELEPHONE_NUMBER_OTHER, this);
    auto fax_edit = new StringOtherEdit(ui->fax_edit, ui->fax_button, ATTRIBUTE_FAX_NUMBER, ATTRIBUTE_OTHER_FAX_NUMBER, this);

    manager_edits = {
        office_edit,
        street_edit,
        city_edit,
        state_edit,
        country_edit,
        telephone_edit,
        fax_edit,
    };

    telephone_edit->set_read_only(true);
    fax_edit->set_read_only(true);

    connect(
        manager_edit, &ManagerEdit::edited,
        this, &ManagedByTabEdit::on_manager_edited);
}

ManagedByTab::~ManagedByTab() {
    delete ui;
}

void ManagedByTabEdit::on_manager_edited() {
    AdInterface ad;
    if (ad_failed(ad, ui->manager_widget)) {
        return;
    }

    load_manager_edits(ad);

    emit edited();
}

void ManagedByTabEdit::load(AdInterface &ad, const AdObject &object) {
    manager_edit->load(ad, object);

    // NOTE: load AFTER loading manager! because manager
    // edits use current value of manager edit
    load_manager_edits(ad);
}

bool ManagedByTabEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool success = manager_edit->apply(ad, dn);

    return success;
}

void ManagedByTabEdit::load_manager_edits(AdInterface &ad) {
    const QString manager = manager_edit->get_manager();

    if (!manager.isEmpty()) {
        const AdObject manager_object = ad.search_object(manager);
        AttributeEdit::load(manager_edits, ad, manager_object);
    } else {
        AdObject empty_object;
        AttributeEdit::load(manager_edits, ad, empty_object);
    }
}
