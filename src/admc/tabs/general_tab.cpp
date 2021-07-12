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

#include "tabs/general_tab.h"

#include "adldap.h"
#include "edits/country_edit.h"
#include "edits/group_scope_edit.h"
#include "edits/group_type_edit.h"
#include "edits/string_edit.h"
#include "edits/string_other_edit.h"

#include <QFormLayout>
#include <QFrame>
#include <QLabel>

// TODO: other object types also have special general tab versions, like top level domain object for example. Find out all of them and implement them

GeneralTab::GeneralTab(const AdObject &object) {
    auto name_label = new QLabel();
    const QString name = object.get_string(ATTRIBUTE_NAME);
    name_label->setText(name);

    if (object.is_empty()) {
        name_label->setText(tr("Failed to load object information. Check your connection."));
    }

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
        StringEdit::make_many(attributes, CLASS_USER, &edits, this);

        new StringOtherEdit(ATTRIBUTE_TELEPHONE_NUMBER, ATTRIBUTE_TELEPHONE_NUMBER_OTHER, CLASS_USER, &edits, this);
        new StringOtherEdit(ATTRIBUTE_WWW_HOMEPAGE, ATTRIBUTE_WWW_HOMEPAGE_OTHER, CLASS_USER, &edits, this);
    } else if (object.is_class(CLASS_OU)) {
        const QList<QString> attributes = {
            ATTRIBUTE_DESCRIPTION,
            ATTRIBUTE_STREET,
            ATTRIBUTE_CITY,
            ATTRIBUTE_STATE,
            ATTRIBUTE_POSTAL_CODE,
        };

        StringEdit::make_many(attributes, CLASS_OU, &edits, this);

        new CountryEdit(&edits, this);
    } else if (object.is_class(CLASS_COMPUTER)) {
        auto sama_edit = new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, CLASS_COMPUTER, &edits, this);
        sama_edit->set_read_only(true);

        auto dns_edit = new StringEdit(ATTRIBUTE_DNS_HOST_NAME, CLASS_COMPUTER, &edits, this);
        dns_edit->set_read_only(true);

        new StringEdit(ATTRIBUTE_DESCRIPTION, CLASS_COMPUTER, &edits, this);
        new StringEdit(ATTRIBUTE_LOCATION, CLASS_COMPUTER, &edits, this);

        // TODO: more string edits for: site (probably just site?), dc type (no idea)
    } else if (object.is_class(CLASS_GROUP)) {
        const QList<QString> string_attributes = {
            ATTRIBUTE_SAMACCOUNT_NAME,
            ATTRIBUTE_DESCRIPTION,
            ATTRIBUTE_MAIL,
            ATTRIBUTE_INFO,
        };
        StringEdit::make_many(string_attributes, CLASS_GROUP, &edits, this);

        auto scope_edit = new GroupScopeEdit(&edits, this);

        auto type_edit = new GroupTypeEdit(&edits, this);

        const bool is_critical_system_object = object.get_bool(ATTRIBUTE_IS_CRITICAL_SYSTEM_OBJECT);
        if (is_critical_system_object) {
            scope_edit->set_read_only(true);
            type_edit->set_read_only(true);
        }
    } else if (object.is_class(CLASS_CONTAINER)) {
        new StringEdit(ATTRIBUTE_DESCRIPTION, CLASS_GROUP, &edits, this);
    }

    edits_add_to_layout(edits, edits_layout);
    edits_connect_to_tab(edits, this);
}
