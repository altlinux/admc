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
#include "attribute_edit.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QMap>

// TODO: add "other" values for phone and homepage. This looks like the attribute is multi-valued but couldn't see that it is in attrib editor for some reason.
// TODO: show icon if needed

// NOTE: https://ldapwiki.com/wiki/MMC%20Account%20Tab

GeneralTab::GeneralTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    title = tr("General");

    name_label = new QLabel();

    const auto edits_layout = new QGridLayout();

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(name_label);
    top_layout->addLayout(edits_layout);

    const QList<QString> attributes = {
        ATTRIBUTE_DESCRIPTION,
        ATTRIBUTE_FIRST_NAME,
        ATTRIBUTE_LAST_NAME,
        ATTRIBUTE_DISPLAY_NAME,
        ATTRIBUTE_INITIALS,
        ATTRIBUTE_MAIL,
        ATTRIBUTE_OFFICE,
        ATTRIBUTE_TELEPHONE_NUMBER,
        ATTRIBUTE_WWW_HOMEPAGE
    };

    QMap<QString, StringEdit *> string_edits;
    make_string_edits(attributes, &string_edits);

    for (auto attribute : attributes) {
        auto edit = string_edits[attribute];
        edit->add_to_layout(edits_layout);
        edits.append(edit);
    }

    connect_edits_to_details(edits, details);

    QLineEdit *full_name_edit = string_edits[ATTRIBUTE_DISPLAY_NAME]->edit;
    QLineEdit *first_name_edit = string_edits[ATTRIBUTE_FIRST_NAME]->edit;
    QLineEdit *last_name_edit = string_edits[ATTRIBUTE_LAST_NAME]->edit;
    autofill_full_name(full_name_edit, first_name_edit, last_name_edit);
}

void GeneralTab::apply() {
    apply_attribute_edits(edits, target(), ApplyAttributeEditBatch_No, this);
}

bool GeneralTab::accepts_target() const {
    return AdInterface::instance()->has_attributes(target());
}

void GeneralTab::reload_internal() {
    const QString name = AdInterface::instance()->attribute_get(target(), ATTRIBUTE_NAME);
    name_label->setText(name);

    for (auto edit : edits) {
        edit->load(target());
    }
}
