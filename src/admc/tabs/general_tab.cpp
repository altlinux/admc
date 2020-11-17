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

#include "tabs/general_tab.h"
#include "ad_interface.h"
#include "edits/attribute_edit.h"
#include "edits/string_edit.h"
#include "edits/string_other_edit.h"
#include "edits/country_edit.h"
#include "edits/group_scope_edit.h"
#include "edits/group_type_edit.h"
#include "utils.h"
#include "ad_config.h"

#include <QLabel>
#include <QFormLayout>
#include <QMap>
#include <QFrame>

// TODO: other object types also have special general tab versions, like top level domain object for example. Find out all of them and implement them

GeneralTab::GeneralTab(const AdObject &object) {   
    auto name_label = new QLabel();
    const QString name = object.get_string(ATTRIBUTE_NAME);
    name_label->setText(name);

    auto line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    auto edits_layout = new QFormLayout();

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(name_label);
    top_layout->addWidget(line);
    top_layout->addLayout(edits_layout);

    if (object.is_class(CLASS_USER)) {
        const QList<QString> attributes = {
            ATTRIBUTE_DESCRIPTION,
            ATTRIBUTE_FIRST_NAME,
            ATTRIBUTE_LAST_NAME,
            ATTRIBUTE_DISPLAY_NAME,
            ATTRIBUTE_INITIALS,
            ATTRIBUTE_MAIL,
            ATTRIBUTE_OFFICE,
        };
        make_string_edits(attributes, CLASS_USER, this, &edits);

        new StringOtherEdit(ATTRIBUTE_TELEPHONE_NUMBER, ATTRIBUTE_TELEPHONE_NUMBER_OTHER, CLASS_USER, this, &edits);
        new StringOtherEdit(ATTRIBUTE_WWW_HOMEPAGE, ATTRIBUTE_WWW_HOMEPAGE_OTHER, CLASS_USER, this, &edits);
    } else if (object.is_class(CLASS_OU)) {
        const QList<QString> attributes = {
            ATTRIBUTE_DESCRIPTION,
            ATTRIBUTE_STREET,
            ATTRIBUTE_CITY,
            ATTRIBUTE_STATE,
            ATTRIBUTE_POSTAL_CODE,
        };

        make_string_edits(attributes, CLASS_OU, this, &edits);

        new CountryEdit(this, &edits);
    } else if (object.is_class(CLASS_COMPUTER)) {
        auto sama_edit = new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, CLASS_COMPUTER, this, &edits);
        sama_edit->set_read_only(true);
        
        auto dns_edit = new StringEdit(ATTRIBUTE_DNS_HOST_NAME, CLASS_COMPUTER, this, &edits);
        dns_edit->set_read_only(true);
        
        new StringEdit(ATTRIBUTE_DESCRIPTION, CLASS_COMPUTER, this, &edits);

        // TODO: more string edits for: site (probably just site?), dc type (no idea)
    } else if (object.is_class(CLASS_GROUP)) {
        const QList<QString> string_attributes = {
            ATTRIBUTE_SAMACCOUNT_NAME,
            ATTRIBUTE_DESCRIPTION,
            ATTRIBUTE_MAIL,
            ATTRIBUTE_INFO,
        };
        make_string_edits(string_attributes, CLASS_GROUP, this, &edits);
        
        auto scope_edit = new GroupScopeEdit(this, &edits);
        
        auto type_edit = new GroupTypeEdit(this, &edits);

        const bool is_critical_system_object = (object.contains(ATTRIBUTE_IS_CRITICAL_SYSTEM_OBJECT) && object.get_bool(ATTRIBUTE_IS_CRITICAL_SYSTEM_OBJECT));
        if (is_critical_system_object) {
            scope_edit->set_read_only(true);
            type_edit->set_read_only(true);
        }
    } else if (object.is_class(CLASS_CONTAINER)) {
        new StringEdit(ATTRIBUTE_DESCRIPTION, CLASS_GROUP, this, &edits);
    }

    edits_add_to_layout(edits, edits_layout);
    edits_connect_to_tab(edits, this);  
}
