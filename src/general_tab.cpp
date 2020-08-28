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
#include "edits/country_edit.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QMap>
#include <QStackedWidget>
#include <QFrame>

// TODO: disable scope/type edit for system type groups, there are other object specific general tabs, for domain top level object for example

GeneralTab::GeneralTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    name_label = new QLabel();

    stacked_widget = new QStackedWidget();

    QGridLayout *layout_for_type[GeneralTabType_COUNT];

    // NOTE: only creating widgets to hold the layout, because can't add layouts directly
    for (int i = 0; i < GeneralTabType_COUNT; i++) {
        auto widget = new QWidget();
        auto layout = new QGridLayout();
        
        layout_for_type[i] = layout;

        widget->setLayout(layout);
        stacked_widget->addWidget(widget);
    }

    auto line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(name_label);
    top_layout->addWidget(line);
    top_layout->addWidget(stacked_widget);

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
        make_string_edits(attributes, &string_edits);

        for (auto attribute : attributes) {
            auto edit = string_edits[attribute];
            edits->append(edit);
        }

        autofill_full_name(string_edits);
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
        make_string_edits(attributes, &string_edits);

        for (auto attribute : attributes) {
            auto edit = string_edits[attribute];
            edits->append(edit);
        }

        edits->append(new CountryEdit());
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
        make_string_edits(attributes, &string_edits);

        string_edits[ATTRIBUTE_SAMACCOUNT_NAME]->set_read_only(EditReadOnly_Yes);
        string_edits[ATTRIBUTE_DNS_HOST_NAME]->set_read_only(EditReadOnly_Yes);

        for (auto attribute : attributes) {
            auto edit = string_edits[attribute];
            edits->append(edit);
        }

        // TODO: more string edits for: site (probably just site?), dc type (no idea)
    }

    // Group
    {
        QList<AttributeEdit *> *edits = &(edits_for_type[GeneralTabType_Group]);

        edits->append(new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME));
        edits->append(new StringEdit(ATTRIBUTE_DESCRIPTION));
        edits->append(new StringEdit(ATTRIBUTE_MAIL));
        edits->append(new StringEdit(ATTRIBUTE_INFO));

        edits->append(new GroupScopeEdit());
        edits->append(new GroupTypeEdit());
    }

    // Container
    {
        QList<AttributeEdit *> *edits = &(edits_for_type[GeneralTabType_Container]);

        edits->append(new StringEdit(ATTRIBUTE_DESCRIPTION));
    }

    for (int i = 0; i < GeneralTabType_COUNT; i++) {
        QGridLayout *layout = layout_for_type[i];

        layout_attribute_edits(edits_for_type[i], layout);
        connect_edits_to_tab(edits_for_type[i], this);  
    }
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
    return AdInterface::instance()->has_attributes(target());
}

void GeneralTab::reload() {
    const QString name = AdInterface::instance()->attribute_get(target(), ATTRIBUTE_NAME);
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

    // Hide all stacked widgets
    for (int i = 0; i < GeneralTabType_COUNT; i++) {
        QWidget *widget = stacked_widget->widget(i);
        widget->hide();
    }

    // Show current type's widget
    const int current_widget_index = (int)type;
    QWidget *current_widget = stacked_widget->widget(current_widget_index);
    current_widget->show();
}
