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

#include "details_dialog.h"
#include "tabs/details_tab.h"
#include "tabs/attributes_tab.h"
#include "tabs/membership_tab.h"
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

DetailsDialog *DetailsDialog::docked_instance = nullptr;

QWidget *DetailsDialog::get_docked_container() {
    static QWidget *docked_container =
    []() {
        auto out = new QWidget();

        auto layout = new QVBoxLayout();
        out->setLayout(layout);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        return out;
    }();

    return docked_container;
}

void DetailsDialog::open_for_target(const QString &target) {
    const bool is_docked = SETTINGS()->get_bool(BoolSetting_DetailsIsDocked);

    if (is_docked) {
        // Close (delete) previous docked instance
        if (docked_instance != nullptr) {
            docked_instance->close();
        }

        docked_instance = new DetailsDialog(target, false);

        // Add new docked instance to container layout
        QWidget *docked_container = get_docked_container();
        QLayout *docked_layout = docked_container->layout();
        docked_layout->addWidget(docked_instance);
    } else {
        auto dialog = new DetailsDialog(target, true);
        dialog->open();
    }
}

DetailsDialog::DetailsDialog(const QString &target_arg, const bool is_floating_instance_arg)
: QDialog()
{
    target = target_arg;
    is_floating_instance = is_floating_instance_arg;

    setAttribute(Qt::WA_DeleteOnClose);

    if (is_floating_instance) {
        resize(600, 700);
    }

    title_label = new QLabel(this);
    tab_widget = new QTabWidget(this);

    button_box = new QDialogButtonBox(QDialogButtonBox::Apply |  QDialogButtonBox::Cancel, this);

    const auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->addWidget(title_label);
    layout->addWidget(tab_widget);
    layout->addWidget(button_box);

    const AdObject object = AD()->search_object(target);

    // TODO: is this actually possible and what should happen, currently leaving the dialog blank which might be enough.
    if (object.is_empty()) {
        return;
    }

    const QString name = object.get_string(ATTRIBUTE_NAME);
    const QString title_text = name.isEmpty() ? tr("Details") : QString(tr("%1 Details")).arg(name);
    title_label->setText(title_text);

    QList<QString> titles;

    // Create new tabs
    const auto add_tab =
    [this, &titles](DetailsTab *tab, const QString &title) {
        tabs.append(tab);
        tab_widget->addTab(tab, title);
        titles.append(title);
    };

    add_tab(new GeneralTab(object), tr("General"));
    add_tab(new ObjectTab(), tr("Object"));
    add_tab(new AttributesTab(), tr("Attributes"));
    if (object.is_class(CLASS_USER)) {
        add_tab(new AccountTab(), tr("Account"));
        add_tab(new AddressTab(), tr("Address"));
    }
    if (object.is_class(CLASS_GROUP)) {
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
    if (object.is_class(CLASS_OU)) {
        // TODO: not sure which object classes can have gplink, for now only know of OU's.
        add_tab(new GroupPolicyTab(), tr("Group policy"));
    }
    if (object.is_class(CLASS_GP_CONTAINER)) {
        add_tab(new GpoLinksTab(), tr("Links to"));
    }

    for (auto tab : tabs) {
        tab->load(object);
        tab->reset();
    }

    for (auto tab : tabs) {
        connect(
            tab, &DetailsTab::edited,
            this, &DetailsDialog::on_tab_edited);
    }
    on_tab_edited();

    for (auto tab : tabs) {
        if (tab->changed()) {
            const QString title = titles[tabs.indexOf(tab)];
            printf("ERROR: a newly created tab %s is in changed() state! Something must be wrong with edits.\n", qPrintable(title));
        }
    }

    connect(
        button_box->button(QDialogButtonBox::Apply), &QPushButton::clicked,
        this, &DetailsDialog::on_apply);
    connect(
        button_box->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
        this, &DetailsDialog::on_cancel);
    connect(
        AD(), &AdInterface::modified,
        this, &DetailsDialog::on_ad_modified);

    const BoolSettingSignal *docked_setting = SETTINGS()->get_bool_signal(BoolSetting_DetailsIsDocked);
    connect(
        docked_setting, &BoolSettingSignal::changed,
        this, &DetailsDialog::on_docked_setting_changed);
    on_docked_setting_changed();
}

void DetailsDialog::on_docked_setting_changed() {
    const bool is_docked = SETTINGS()->get_bool(BoolSetting_DetailsIsDocked);

    if (is_floating_instance) {
        // Close dialog if changed to docked
        if (is_docked) {
            QDialog::close();
        }
    } else {
        // Hide/show docked instance depending on docked setting
        get_docked_container()->setVisible(is_docked);
    }
}

QString DetailsDialog::get_target() const {
    return target;
}

void DetailsDialog::on_apply() {
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

    const AdObject object = AD()->search_object(target);
    for (auto tab : tabs) {
        tab->load(object);
    }
}

void DetailsDialog::on_cancel() {
    for (auto tab : tabs) {
        tab->reset();
    }

    // Call slot to reset to unchanged state
    on_tab_edited();
}

void DetailsDialog::on_tab_edited() {
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

void DetailsDialog::on_ad_modified() {
    const AdObject object = AD()->search_object(target);

    if (object.is_empty()) {
        close();
        docked_instance = nullptr;
    } else {
        for (auto tab : tabs) {
            tab->load(object);
        }
    }
}
