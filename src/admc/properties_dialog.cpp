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

#include "properties_dialog.h"

#include "adldap.h"
#include "console_types/console_object.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "tab_widget.h"
#include "tabs/account_tab.h"
#include "tabs/address_tab.h"
#include "tabs/attributes_tab.h"
#include "tabs/general_tab.h"
#include "tabs/gpo_links_tab.h"
#include "tabs/group_policy_tab.h"
#include "tabs/managed_by_tab.h"
#include "tabs/membership_tab.h"
#include "tabs/object_tab.h"
#include "tabs/organization_tab.h"
#include "tabs/profile_tab.h"
#include "tabs/properties_tab.h"
#include "tabs/security_tab.h"
#include "tabs/telephones_tab.h"
#include "tabs/os_tab.h"
#include "tabs/delegation_tab.h"
#include "utils.h"

#include <QAbstractItemView>
#include <QAction>
#include <QDebug>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

PropertiesDialog *PropertiesDialog::open_for_target(const QString &target) {
    if (target.isEmpty()) {
        return nullptr;
    }

    show_busy_indicator();

    static QHash<QString, PropertiesDialog *> instances;

    const bool dialog_already_open_for_this_target = instances.contains(target);

    PropertiesDialog *dialog;

    if (dialog_already_open_for_this_target) {
        // Focus already open dialog
        dialog = instances[target];
        dialog->raise();
        dialog->setFocus();
    } else {
        // Make new dialog for this target
        dialog = new PropertiesDialog(target);

        instances[target] = dialog;
        connect(
            dialog, &QDialog::finished,
            [target]() {
                instances.remove(target);
            });

        dialog->show();
    }

    hide_busy_indicator();

    return dialog;
}

void PropertiesDialog::open_when_view_item_activated(QAbstractItemView *view, const int dn_role) {
    connect(
        view, &QAbstractItemView::doubleClicked,
        [dn_role](const QModelIndex &index) {
            const QString dn = index.data(dn_role).toString();

            open_for_target(dn);
        });
}

PropertiesDialog::PropertiesDialog(const QString &target_arg)
: QDialog() {
    target = target_arg;
    is_modified = false;

    setAttribute(Qt::WA_DeleteOnClose);

    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    apply_button = button_box->addButton(QDialogButtonBox::Apply);
    reset_button = button_box->addButton(QDialogButtonBox::Reset);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);

    // Make ok button "auto default", which means that
    // pressing enter will press ok button
    cancel_button->setAutoDefault(false);
    reset_button->setAutoDefault(false);
    apply_button->setAutoDefault(false);
    ok_button->setAutoDefault(true);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setSpacing(0);

    const QString title = [&]() {
        const QString target_name = dn_get_name(target_arg);
        
        if (!target_name.isEmpty()) {
            return QString(tr("%1 Properties")).arg(target_name);
        } else {
            return tr("Properties");
        }
    }();
    setWindowTitle(title);

    AdObject object;

    AdInterface ad;
    if (!ad_failed(ad)) {
        object = ad.search_object(target);
    }

    auto tab_widget = new TabWidget();

    layout->addWidget(tab_widget);
    layout->addWidget(button_box);

    // Create new tabs
    const auto add_tab = [this, tab_widget](PropertiesTab *tab, const QString &tab_title) {
        tabs.append(tab);
        tab_widget->add_tab(tab, tab_title);
    };

    add_tab(new GeneralTab(object), tr("General"));

    const bool advanced_view_ON = settings_get_bool(SETTING_advanced_features);
    if (advanced_view_ON) {
        add_tab(new ObjectTab(), tr("Object"));

        attributes_tab = new AttributesTab();
        add_tab(attributes_tab, tr("Attributes"));
    } else {
        attributes_tab = nullptr;
    }

    if (object.is_class(CLASS_USER)) {
        add_tab(new AccountTab(ad), tr("Account"));
        add_tab(new AddressTab(), tr("Address"));
        add_tab(new OrganizationTab(), tr("Organization"));
        add_tab(new TelephonesTab(), tr("Telephones"));
        add_tab(new ProfileTab(), tr("Profile"));

        if (advanced_view_ON) {
            add_tab(new SecurityTab(), tr("Security"));
        }
    }
    if (object.is_class(CLASS_GROUP)) {
        add_tab(new MembersTab(), tr("Members"));
    }
    if (object.is_class(CLASS_USER)) {
        add_tab(new MemberOfTab(), tr("Member of"));
    }

    if (object.is_class(CLASS_OU)) {
        add_tab(new ManagedByTab(), tr("Managed by"));
    }

    if (object.is_class(CLASS_OU) || object.is_class(CLASS_DOMAIN)) {
        add_tab(new GroupPolicyTab(), tr("Group policy"));
    }

    if (object.is_class(CLASS_GP_CONTAINER)) {
        add_tab(new GpoLinksTab(), tr("Links to"));
    }

    if (object.is_class(CLASS_COMPUTER)) {
        add_tab(new OSTab(), tr("Operating System"));
        add_tab(new DelegationTab(), tr("Delegation"));
        add_tab(new MemberOfTab(), tr("Member of"));
        add_tab(new ManagedByTab(), tr("Managed by"));
    }

    for (auto tab : tabs) {
        connect(
            tab, &PropertiesTab::edited,
            this, &PropertiesDialog::on_edited);
    }

    if (ad.is_connected()) {
        reset_internal(ad);
    }

    settings_setup_dialog_geometry(SETTING_properties_dialog_geometry, this);
    
    connect(
        ok_button, &QPushButton::clicked,
        this, &PropertiesDialog::ok);
    connect(
        apply_button, &QPushButton::clicked,
        this, &PropertiesDialog::apply);
    connect(
        reset_button, &QPushButton::clicked,
        this, &PropertiesDialog::reset);
    connect(
        cancel_button, &QPushButton::clicked,
        this, &PropertiesDialog::reject);
    connect(
        tab_widget, &TabWidget::current_changed,
        this, &PropertiesDialog::on_current_tab_changed);
}

void PropertiesDialog::on_current_tab_changed(QWidget *prev_tab, QWidget *new_tab) {
    const bool switching_to_or_from_attributes = (prev_tab == attributes_tab || new_tab == attributes_tab);
    const bool need_to_open_dialog = (switching_to_or_from_attributes && is_modified);
    if (!need_to_open_dialog) {
        return;
    }

    // Open dialog which let's user choose whether to
    // apply changes and move to new tab or stay on
    // current tab
    auto dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    const QString explanation_text = [&]() {
        if (new_tab == attributes_tab) {
            return tr("You're switching to attributes tab, while another tab has unapplied changes. Choose to apply or discard those changes.");
        } else {
            return tr("You're switching from attributes tab, while it has unapplied changes. Choose to apply or discard those changes.");
        }
    }();
    auto explanation_label = new QLabel(explanation_text);

    auto button_box = new QDialogButtonBox();
    button_box->addButton(tr("Apply current changes"), QDialogButtonBox::AcceptRole);
    button_box->addButton(tr("Discard changes"), QDialogButtonBox::RejectRole);

    connect(
        button_box, &QDialogButtonBox::accepted,
        dialog, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        dialog, &QDialog::reject);

    const auto layout = new QVBoxLayout();
    dialog->setLayout(layout);
    layout->addWidget(explanation_label);
    layout->addWidget(button_box);

    connect(
        dialog, &QDialog::accepted,
        [&]() {
            AdInterface ad;

            if (ad_connected(ad)) {
                apply_internal(ad);

                // NOTE: have to reset for attributes/other tab
                // to load updates
                reset_internal(ad);
            }
        });

    connect(
        dialog, &QDialog::rejected,
        [&]() {
            reset();
        });

    dialog->open();
}

void PropertiesDialog::ok() {
    const bool success = apply();

    if (success) {
        accept();
    }
}

bool PropertiesDialog::apply() {
    AdInterface ad;
    if (ad_connected(ad)) {
        return apply_internal(ad);
    } else {
        return false;
    }
}

void PropertiesDialog::reset() {
    AdInterface ad;
    if (ad_connected(ad)) {
        reset_internal(ad);
    }
}

bool PropertiesDialog::apply_internal(AdInterface &ad) {
    for (auto tab : tabs) {
        const bool verify_success = tab->verify(ad, target);
        if (!verify_success) {
            return false;
        }
    }

    show_busy_indicator();

    bool total_apply_success = true;

    for (auto tab : tabs) {
        const bool apply_success = tab->apply(ad, target);
        if (!apply_success) {
            total_apply_success = false;
        }
    }

    g_status()->display_ad_messages(ad, this);

    if (total_apply_success) {
        apply_button->setEnabled(false);
        reset_button->setEnabled(false);
        is_modified = false;
    }

    hide_busy_indicator();

    emit applied();

    return total_apply_success;
}

void PropertiesDialog::reset_internal(AdInterface &ad) {
    const AdObject object = ad.search_object(target);

    for (auto tab : tabs) {
        tab->load(ad, object);
    }

    apply_button->setEnabled(false);
    reset_button->setEnabled(false);
    is_modified = false;

    g_status()->display_ad_messages(ad, this);
}

void PropertiesDialog::on_edited() {
    apply_button->setEnabled(true);
    reset_button->setEnabled(true);
    is_modified = true;
}
