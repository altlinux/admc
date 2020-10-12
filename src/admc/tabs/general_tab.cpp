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
#include "edits/string_multi_edit.h"
#include "edits/country_edit.h"
#include "edits/group_scope_edit.h"
#include "edits/group_type_edit.h"
#include "utils.h"
#include "ad_config.h"

#include <QLabel>
#include <QGridLayout>
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

    auto edits_layout = new QGridLayout();

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(name_label);
    top_layout->addWidget(line);
    top_layout->addLayout(edits_layout);

    QMap<QString, StringEdit *> string_edits;

    if (object.is_user()) {
        const QList<QString> attributes = {
            ATTRIBUTE_DESCRIPTION,
            ATTRIBUTE_FIRST_NAME,
            ATTRIBUTE_LAST_NAME,
            ATTRIBUTE_DISPLAY_NAME,
            ATTRIBUTE_INITIALS,
            ATTRIBUTE_MAIL,
            ATTRIBUTE_OFFICE,
        };
        make_string_edits(attributes, CLASS_USER, this, &string_edits, &edits);

        edits.append(new StringMultiEdit(ATTRIBUTE_TELEPHONE_NUMBER, ATTRIBUTE_TELEPHONE_NUMBER_OTHER, CLASS_USER, this));
        edits.append(new StringMultiEdit(ATTRIBUTE_WWW_HOMEPAGE, ATTRIBUTE_WWW_HOMEPAGE_OTHER, CLASS_USER, this));
    } else if (object.is_ou()) {
        const QList<QString> attributes = {
            ATTRIBUTE_DESCRIPTION,
            ATTRIBUTE_STREET,
            ATTRIBUTE_CITY,
            ATTRIBUTE_STATE,
            ATTRIBUTE_POSTAL_CODE,
        };

        make_string_edits(attributes, CLASS_OU, this, &string_edits, &edits);

        edits.append(new CountryEdit(this));
    } else if (object.is_computer()) {
        const QList<QString> string_attributes = {
            ATTRIBUTE_SAMACCOUNT_NAME,
            ATTRIBUTE_DNS_HOST_NAME,
            ATTRIBUTE_DESCRIPTION,
        };
        make_string_edits(string_attributes, CLASS_COMPUTER, this, &string_edits, &edits);

        string_edits[ATTRIBUTE_SAMACCOUNT_NAME]->set_read_only(true);
        string_edits[ATTRIBUTE_DNS_HOST_NAME]->set_read_only(true);

        // TODO: more string edits for: site (probably just site?), dc type (no idea)
    } else if (object.is_group()) {
        const QList<QString> string_attributes = {
            ATTRIBUTE_SAMACCOUNT_NAME,
            ATTRIBUTE_DESCRIPTION,
            ATTRIBUTE_MAIL,
            ATTRIBUTE_INFO,
        };
        make_string_edits(string_attributes, CLASS_GROUP, this, &string_edits, &edits);

        edits.append(new GroupScopeEdit(this));
        edits.append(new GroupTypeEdit(this));
    } else if (object.is_container()) {
        make_string_edit(ATTRIBUTE_DESCRIPTION, CLASS_GROUP, this, &string_edits, &edits);
    }

    StringEdit::setup_autofill(string_edits.values());

    edits_add_to_layout(edits, edits_layout);
    edits_connect_to_tab(edits, this);  
}
