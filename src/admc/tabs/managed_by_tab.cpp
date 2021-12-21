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

    new ManagedByTabEdit(edit_list, ui, this);
}

ManagedByTabEdit::ManagedByTabEdit(QList<AttributeEdit *> *edit_list, Ui::ManagedByTab *ui_arg, QObject *parent)
: AttributeEdit(edit_list, parent) {
    ui = ui_arg;

    manager_edit = new ManagerEdit(ui->manager_widget, ATTRIBUTE_MANAGED_BY, edit_list, this);

    new StringEdit(ui->office_edit, ATTRIBUTE_OFFICE, &manager_edits, this);
    new StringEdit(ui->street_edit, ATTRIBUTE_STREET, &manager_edits, this);
    new StringEdit(ui->city_edit, ATTRIBUTE_CITY, &manager_edits, this);
    new StringEdit(ui->state_edit, ATTRIBUTE_STATE, &manager_edits, this);

    new CountryEdit(ui->country_combo, &manager_edits, this);

    new StringOtherEdit(ui->telephone_edit, ui->telephone_button, ATTRIBUTE_TELEPHONE_NUMBER, ATTRIBUTE_TELEPHONE_NUMBER_OTHER, &manager_edits, this);
    new StringOtherEdit(ui->fax_edit, ui->fax_button, ATTRIBUTE_FAX_NUMBER, ATTRIBUTE_OTHER_FAX_NUMBER, &manager_edits, this);

    AttributeEdit::set_read_only(manager_edits, true);

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
}

void ManagedByTabEdit::load(AdInterface &ad, const AdObject &object) {
    manager_edit->load(ad, object);

    // NOTE: load AFTER loading manager! because manager
    // edits use current value of manager edit
    load_manager_edits(ad);
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

bool ManagedByTabEdit::apply(AdInterface &ad, const QString &target) {
    UNUSED_ARG(ad);
    UNUSED_ARG(target);

    return true;
}

void ManagedByTabEdit::set_read_only(const bool read_only) {
    UNUSED_ARG(read_only);
}
