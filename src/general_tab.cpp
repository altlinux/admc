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

#include "general_tab.h"
#include "ad_interface.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QDateTime>
#include <QButtonGroup>
#include <QCalendarWidget>
#include <QDialog>

// TODO: add "other" values for phone and homepage. This looks like the attribute is multi-valued but couldn't see that it is in attrib editor for some reason.
// TODO: show icon if needed

// NOTE: https://ldapwiki.com/wiki/MMC%20Account%20Tab

GeneralTab::GeneralTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    title = tr("General");

    name_label = new QLabel(this);

    // Put labels in one vertical layout and edits in another
    // So that they are all aligned and get enough space
    const auto top_layout = new QVBoxLayout(this);
    top_layout->addWidget(name_label);

    const auto attributes_layout = new QHBoxLayout();
    top_layout->insertLayout(-1, attributes_layout);

    const auto label_layout = new QVBoxLayout();
    const auto edit_layout = new QVBoxLayout();
    attributes_layout->insertLayout(-1, label_layout);
    attributes_layout->insertLayout(-1, edit_layout);

    auto add_edit =
    [this, label_layout, edit_layout](const QString &attribute, const QString &label_text) {
        add_attribute_edit(attribute, label_text, label_layout, edit_layout, AttributeEditType_Editable);
    };

    add_edit(ATTRIBUTE_DISPLAY_NAME, tr("Display name:"));
    add_edit(ATTRIBUTE_DESCRIPTION, tr("Description:"));
    add_edit(ATTRIBUTE_FIRST_NAME, tr("First name"));
    add_edit(ATTRIBUTE_INITIALS, tr("Initials:"));
    add_edit(ATTRIBUTE_MAIL, tr("Email:"));
    add_edit(ATTRIBUTE_OFFICE, tr("Office:"));
    add_edit(ATTRIBUTE_SN, tr("Last name:"));
    add_edit(ATTRIBUTE_TELEPHONE_NUMBER, tr("Phone:"));
    add_edit(ATTRIBUTE_WWW_HOMEPAGE, tr("Homepage:"));
}

bool GeneralTab::accepts_target() const {
    return AdInterface::instance()->has_attributes(target());
}

void GeneralTab::reload_internal() {
    const QString name = AdInterface::instance()->attribute_get(target(), ATTRIBUTE_NAME);
    name_label->setText(name);
}
