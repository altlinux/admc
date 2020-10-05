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
#include "edits/attribute_edit.h"
#include "edits/string_edit.h"
#include "edits/country_edit.h"
#include "edits/group_scope_edit.h"
#include "edits/group_type_edit.h"
#include "utils.h"
#include "server_configuration.h"

#include <QLabel>
#include <QGridLayout>
#include <QMap>
#include <QFrame>

// TODO: other object types also have special general tab versions, like top level domain object for example. Find out all of them and implement them

enum GeneralTabType {
    GeneralTabType_Default,
    GeneralTabType_User,
    GeneralTabType_OU,
    GeneralTabType_Computer,
    GeneralTabType_Group,
    GeneralTabType_Container,
    GeneralTabType_COUNT
};

GeneralTab::GeneralTab()
: DetailsTab()
{   
    name_label = new QLabel();

    auto line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    edits_layout = new QGridLayout();

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(name_label);
    top_layout->addWidget(line);
    top_layout->addLayout(edits_layout);
}

bool GeneralTab::changed() const {
    return any_edits_changed(edits);
}

bool GeneralTab::verify() {
    return verify_attribute_edits(edits, this);
}

void GeneralTab::apply(const QString &target) {
    apply_attribute_edits(edits, target, this);
}

bool GeneralTab::accepts_target(const Attributes &) const {
    return true;
}

void GeneralTab::load(const QString &target, const Attributes &attributes) {
    const QString name = attribute_get_string(attributes, ATTRIBUTE_NAME);
    name_label->setText(name);

    const GeneralTabType type =
    [attributes]() {
        const bool is_user = object_is_user(attributes);
        const bool is_ou = object_is_ou(attributes);
        const bool is_computer = object_is_computer(attributes);
        const bool is_group = object_is_group(attributes);
        const bool is_container = object_is_container(attributes);

        if (is_computer) {
            return GeneralTabType_Computer;
        } else if (is_user) {
            return GeneralTabType_User;
        } else if (is_ou) {
            return GeneralTabType_OU;
        } else if (is_group) {
            return GeneralTabType_Group;
        } else if (is_container) {
            return GeneralTabType_Container;
        } else {
            return GeneralTabType_Default;
        }
    }();

    switch (type) {
        case GeneralTabType_User: {
            const QList<QString> string_attributes = {
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
            make_string_edits(string_attributes, CLASS_USER, &string_edits, &edits, this);

            setup_string_edit_autofills(string_edits);

            break;
        }
        case GeneralTabType_OU: {
            const QList<QString> string_attributes = {
                ATTRIBUTE_DESCRIPTION,
                ATTRIBUTE_STREET,
                ATTRIBUTE_CITY,
                ATTRIBUTE_STATE,
                ATTRIBUTE_POSTAL_CODE,
            };

            QMap<QString, StringEdit *> string_edits;
            make_string_edits(string_attributes, CLASS_OU, &string_edits, &edits, this);

            edits.append(new CountryEdit(this));

            break;
        }
        case GeneralTabType_Computer: {
            const QList<QString> string_attributes = {
                ATTRIBUTE_SAMACCOUNT_NAME,
                ATTRIBUTE_DNS_HOST_NAME,
                ATTRIBUTE_DESCRIPTION,
            };

            QMap<QString, StringEdit *> string_edits;
            make_string_edits(string_attributes, CLASS_COMPUTER, &string_edits, &edits, this);

            string_edits[ATTRIBUTE_SAMACCOUNT_NAME]->set_read_only(EditReadOnly_Yes);
            string_edits[ATTRIBUTE_DNS_HOST_NAME]->set_read_only(EditReadOnly_Yes);

            // TODO: more string edits for: site (probably just site?), dc type (no idea)

            break;
        }
        case GeneralTabType_Group: {
            // TODO: use make_string_edits()
            edits.append(new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, CLASS_GROUP, this));
            edits.append(new StringEdit(ATTRIBUTE_DESCRIPTION, CLASS_GROUP, this));
            edits.append(new StringEdit(ATTRIBUTE_MAIL, CLASS_GROUP, this));
            edits.append(new StringEdit(ATTRIBUTE_INFO, CLASS_GROUP, this));

            edits.append(new GroupScopeEdit(this));
            edits.append(new GroupTypeEdit(this));
        
            break;
        }
        case GeneralTabType_Container: {
            edits.append(new StringEdit(ATTRIBUTE_DESCRIPTION, CLASS_CONTAINER, this));

            break;
        }
        default: {
            break;
        }
    }

    layout_attribute_edits(edits, edits_layout);
    connect_edits_to_tab(edits, this);  

    load_attribute_edits(edits, attributes);
}
