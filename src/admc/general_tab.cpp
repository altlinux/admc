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

#include <QVBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QMap>
#include <QStackedLayout>
#include <QFrame>

// TODO: other object types also have special general tab versions, like top level domain object for example. Find out all of them and implement them

GeneralTab::GeneralTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    name_label = new QLabel();

    types_stack = new QStackedLayout();

    QGridLayout *layout_for_type[GeneralTabType_COUNT];

    // NOTE: put grid layout in a vbox layout to group subwidgets neatly
    for (int i = 0; i < GeneralTabType_COUNT; i++) {
        auto grid_layout = new QGridLayout();
        auto wrapper_layout = new QVBoxLayout();
        wrapper_layout->addLayout(grid_layout);
        auto widget = new QWidget();
        widget->setLayout(wrapper_layout);

        layout_for_type[i] = grid_layout;

        types_stack->addWidget(widget);
    }

    auto line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(name_label);
    top_layout->addWidget(line);
    top_layout->addLayout(types_stack);

    // User
    {
        QList<AttributeEdit *> *edits = &(edits_for_type[GeneralTabType_User]);

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
        make_string_edits(attributes, CLASS_USER, &string_edits, edits, this);

        setup_string_edit_autofills(string_edits);
    }

    // OU
    {
        QList<AttributeEdit *> *edits = &(edits_for_type[GeneralTabType_OU]);

        const QList<QString> attributes = {
            ATTRIBUTE_DESCRIPTION,
            ATTRIBUTE_STREET,
            ATTRIBUTE_CITY,
            ATTRIBUTE_STATE,
            ATTRIBUTE_POSTAL_CODE,
        };

        QMap<QString, StringEdit *> string_edits;
        make_string_edits(attributes, CLASS_OU, &string_edits, edits, this);

        edits->append(new CountryEdit(this));
    }

    // Computer
    {
        QList<AttributeEdit *> *edits = &(edits_for_type[GeneralTabType_Computer]);

        const QList<QString> attributes = {
            ATTRIBUTE_SAMACCOUNT_NAME,
            ATTRIBUTE_DNS_HOST_NAME,
            ATTRIBUTE_DESCRIPTION,
        };

        QMap<QString, StringEdit *> string_edits;
        make_string_edits(attributes, CLASS_COMPUTER, &string_edits, edits, this);

        string_edits[ATTRIBUTE_SAMACCOUNT_NAME]->set_read_only(EditReadOnly_Yes);
        string_edits[ATTRIBUTE_DNS_HOST_NAME]->set_read_only(EditReadOnly_Yes);

        // TODO: more string edits for: site (probably just site?), dc type (no idea)
    }

    // Group
    {
        QList<AttributeEdit *> *edits = &(edits_for_type[GeneralTabType_Group]);

        // TODO: use make_string_edits()
        edits->append(new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, CLASS_GROUP, this));
        edits->append(new StringEdit(ATTRIBUTE_DESCRIPTION, CLASS_GROUP, this));
        edits->append(new StringEdit(ATTRIBUTE_MAIL, CLASS_GROUP, this));
        edits->append(new StringEdit(ATTRIBUTE_INFO, CLASS_GROUP, this));

        edits->append(new GroupScopeEdit(this));
        edits->append(new GroupTypeEdit(this));
    }

    // Container
    {
        QList<AttributeEdit *> *edits = &(edits_for_type[GeneralTabType_Container]);

        edits->append(new StringEdit(ATTRIBUTE_DESCRIPTION, CLASS_CONTAINER, this));
    }

    for (int i = 0; i < GeneralTabType_COUNT; i++) {
        QGridLayout *layout = layout_for_type[i];

        layout_attribute_edits(edits_for_type[i], layout);
        connect_edits_to_tab(edits_for_type[i], this);  
    }

    // types_stack->widget(2)->show();

    // const auto test_w = new QWidget();
    // test_w->setLayout(layout_for_type[2]);
    // top_layout->addWidget(test_w);

    // top_layout->addLayout(layout_for_type[2]);
}

bool GeneralTab::changed() const {
    return any_edits_changed(edits_for_type[type]);
}

bool GeneralTab::verify() {
    return verify_attribute_edits(edits_for_type[type], this);
}

void GeneralTab::apply() {
    apply_attribute_edits(edits_for_type[type], target(), this);
}

bool GeneralTab::accepts_target() const {
    return true;
}

void GeneralTab::reload() {
    const QString name = AdInterface::instance()->attribute_get_value(target(), ATTRIBUTE_NAME);
    name_label->setText(name);

    const bool is_user = AdInterface::instance()->is_user(target());
    const bool is_ou = AdInterface::instance()->is_ou(target());
    const bool is_computer = AdInterface::instance()->is_computer(target());
    const bool is_group = AdInterface::instance()->is_group(target());
    const bool is_container = AdInterface::instance()->is_container(target());

    if (is_computer) {
        type = GeneralTabType_Computer;
    } else if (is_user) {
        type = GeneralTabType_User;
    } else if (is_ou) {
        type = GeneralTabType_OU;
    } else if (is_group) {
        type = GeneralTabType_Group;
    } else if (is_container) {
        type = GeneralTabType_Container;
    } else {
        type = GeneralTabType_Default;
    }

    load_attribute_edits(edits_for_type[type], target());

    types_stack->setCurrentIndex((int)type);
}
