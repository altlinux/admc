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

#include "details_widget.h"
#include "tabs/details_tab.h"
#include "tabs/attributes_tab.h"
#include "tabs/members_tab.h"
#include "tabs/account_tab.h"
#include "tabs/general_tab.h"
#include "tabs/address_tab.h"
#include "tabs/object_tab.h"
#include "tabs/group_policy_tab.h"
#include "tabs/gpo_links_tab.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QAction>
#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDebug>

DetailsWidget *DetailsWidget::docked_instance() {
    static DetailsWidget *docked = new DetailsWidget(false);
    return docked;
}

void DetailsWidget::change_target(const QString &new_target) {
    const bool is_docked = SETTINGS()->get_bool(BoolSetting_DetailsIsDocked);

    if (is_docked) {
        auto docked = docked_instance();
        docked->reload(new_target);
    } else {
        static auto floating_instance = new DetailsWidget(true);
        floating_instance->reload(new_target);
        floating_instance->open();
    }
}

DetailsWidget::DetailsWidget(const bool is_floating_instance_arg)
: QDialog()
{
    is_floating_instance = is_floating_instance_arg;

    if (is_floating_instance) {
        resize(400, 700);
    }

    title_label = new QLabel(this);
    tab_widget = new QTabWidget(this);

    button_box = new QDialogButtonBox(QDialogButtonBox::Apply |  QDialogButtonBox::Cancel, this);

    const auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->addWidget(title_label);
    layout->addWidget(tab_widget);
    layout->addWidget(button_box);

    connect(
        button_box->button(QDialogButtonBox::Apply), &QPushButton::clicked,
        this, &DetailsWidget::on_apply);
    connect(
        button_box->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
        this, &DetailsWidget::on_cancel);

    const BoolSettingSignal *docked_setting = SETTINGS()->get_bool_signal(BoolSetting_DetailsIsDocked);
    connect(
        docked_setting, &BoolSettingSignal::changed,
        this, &DetailsWidget::on_docked_setting_changed);
    on_docked_setting_changed();

    reload("");
}

void DetailsWidget::on_docked_setting_changed() {
    const bool is_docked = SETTINGS()->get_bool(BoolSetting_DetailsIsDocked);

    if (is_floating_instance) {
        // Hide floating instance when switching to docked one
        // NOTE: floating instance is NOT shown when switching to non-docked view
        if (is_docked) {
            QDialog::reject();
        }
    } else {
        // Hide/show docked instance depending on docked setting
        setVisible(is_docked);
    }
}

QString DetailsWidget::get_target() const {
    return target;
}

void DetailsWidget::reload(const QString &new_target) {
    target = new_target;

    const AdObject object = AD()->request_all(target);

    if (object.is_empty()) {
        if (is_floating_instance) {
            close();
        } else {
            // Docked details widget can't be closed so it
            // becomes blank
            title_label->setText("");
            button_box->hide();
        }
    } else {
        const QString name = object.get_string(ATTRIBUTE_NAME);
        const QString title_text = name.isEmpty() ? tr("Details") : QString(tr("%1 Details")).arg(name);
        title_label->setText(title_text);

        // Clear old tabs
        tab_widget->clear();

        for (auto tab : tabs) {
            delete tab;
        }
        tabs.clear();

        // Create new tabs
        const auto add_tab =
        [this](DetailsTab *tab, const QString &title) {
            tabs.append(tab);
            tab_widget->addTab(tab, title);

            if (tab->changed()) {
                printf("ERROR: a newly created tab %s is in changed() state! Something must be wrong with edits.\n", qPrintable(title));
            }
        };

        add_tab(new GeneralTab(), tr("General"));
        add_tab(new ObjectTab(), tr("Object"));
        add_tab(new AttributesTab(), tr("Attributes"));
        if (object.is_user()) {
            add_tab(new AccountTab(), tr("Account"));
            add_tab(new AddressTab(), tr("Address"));
        }
        if (object.is_group()) {
            add_tab(new MembersTab(), tr("Members"));
        }
        const bool has_member_of_attribute =
        [object]() {
            const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
            const QList<QString> possible_attributes = ADCONFIG()->get_possible_attributes(object_classes);

            return possible_attributes.contains(ATTRIBUTE_MEMBER_OF);
        }();
        if (has_member_of_attribute) {
            add_tab(new MemberOfTab(), tr("Member of"));
        }
        if (object.is_ou()) {
            // TODO: not sure which object classes can have gplink, for now only know of OU's.
            add_tab(new GroupPolicyTab(), tr("Group policy"));
        }
        if (object.is_policy()) {
            add_tab(new GpoLinksTab(), tr("Links to"));
        }

        for (auto tab : tabs) {
            connect(
                tab, &DetailsTab::edited,
                this, &DetailsWidget::on_tab_edited);
        }

        for (auto tab : tabs) {
            tab->load(object);
            tab->reset();
        }

        // Disable apply/cancel since this is a fresh reload and there are no changes
        button_box->show();
        button_box->setEnabled(false);
    }
}

void DetailsWidget::on_apply() {
    const int errors_index = Status::instance()->get_errors_size();

    bool all_verified = true;
    for (auto tab : tabs) {
        if (tab_widget->indexOf(tab) != -1) {
            const bool verify_success = tab->verify();
            if (!verify_success) {
                all_verified = false;
            }
        }
    }

    if (all_verified) {
        AD()->start_batch();
        for (auto tab : tabs) {
            if (tab_widget->indexOf(tab) != -1) {
                tab->apply(target);
            }
        }
        AD()->end_batch();

        Status::instance()->show_errors_popup(errors_index);
    }

    const AdObject object = AD()->request_all(target);
    for (auto tab : tabs) {
        tab->load(object);
    }
}

void DetailsWidget::on_cancel() {
    for (auto tab : tabs) {
        tab->reset();
    }

    // Call slot to reset to unchanged state
    on_tab_edited();
}

void DetailsWidget::on_tab_edited() {
    // Enable/disable apply and cancel depending on if there are
    // any changes in tabs
    bool any_changed = false;
    for (auto tab : tabs) {
        const int tab_index = tab_widget->indexOf(tab);
        const bool tab_is_active = (tab_index != -1);

        if (tab_is_active) {
            const bool tab_changed = tab->changed();

            if (tab_changed) {
                any_changed = true;
            }

            // Add asterisk to end of tab text if it contains
            // changes
            const QString current_text = tab_widget->tabText(tab_index);
            const QString new_text = set_changed_marker(current_text, tab_changed);
            tab_widget->setTabText(tab_index, new_text);
        }
    }

    button_box->setEnabled(any_changed);
}
