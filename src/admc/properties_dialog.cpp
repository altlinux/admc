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

#include "properties_dialog.h"

#include "tabs/properties_tab.h"
#include "tabs/attributes_tab.h"
#include "tabs/membership_tab.h"
#include "tabs/account_tab.h"
#include "tabs/general_tab.h"
#include "tabs/address_tab.h"
#include "tabs/object_tab.h"
#include "tabs/group_policy_tab.h"
#include "tabs/gpo_links_tab.h"
#include "tabs/organization_tab.h"
#include "tabs/telephones_tab.h"
#include "tabs/profile_tab.h"
#include "tabs/managed_by_tab.h"
#include "tabs/security_tab.h"
#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "utils.h"
#include "tab_widget.h"

#include <QAction>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDebug>
#include <QAbstractItemView>

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

QString PropertiesDialog::display_name() {
    return tr("Properties");
}

PropertiesDialog::PropertiesDialog(const QString &target_arg)
: QDialog()
{
    target = target_arg;
    is_modified = false;

    setAttribute(Qt::WA_DeleteOnClose);

    AdInterface ad;
    if (ad_failed(ad)) {
        close();
        return;
    }

    setMinimumHeight(700);
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

    const AdObject object = ad.search_object(target);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setSpacing(0);

    const QString name = object.get_string(ATTRIBUTE_NAME);
    const QString window_title = name.isEmpty() ? PropertiesDialog::display_name() : QString(tr("\"%1\" Properties")).arg(name);
    setWindowTitle(window_title);

    if (object.is_empty()) {
        auto no_object_label = new QLabel(tr("Object could not be found"));
        layout->addWidget(no_object_label);    
        layout->addWidget(button_box);

        button_box->setEnabled(false);
        
        return;
    }

    auto tab_widget = new TabWidget();
    
    layout->addWidget(tab_widget);
    layout->addWidget(button_box);    

    // Create new tabs
    const auto add_tab =
    [this, tab_widget](PropertiesTab *tab, const QString &title) {
        tabs.append(tab);
        tab_widget->add_tab(tab, title);
    };

    add_tab(new GeneralTab(object), tr("General"));

    // TODO: security tab(didn't make it yet) is also "advanced", so add it here when it gets made
    const bool advanced_view_ON = g_settings->get_bool(BoolSetting_AdvancedFeatures);
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
        add_tab(new SecurityTab(), tr("Security"));
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
        // TODO: not sure which object classes can have gplink, for now only know of OU's.
        add_tab(new GroupPolicyTab(), tr("Group policy"));
    }

    if (object.is_class(CLASS_GP_CONTAINER)) {
        add_tab(new GpoLinksTab(), tr("Links to"));
    }

    for (auto tab : tabs) {
        connect(
            tab, &PropertiesTab::edited,
            this, &PropertiesDialog::on_edited);
    }

    reset();


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

    const QString explanation_text =
    [&]() {
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
            apply();

            // NOTE: have to reset for attributes/other tab
            // to load updates
            reset();
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
    // TODO: handle failure
    AdInterface ad;
    if (ad_failed(ad)) {
        return false;
    }

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

void PropertiesDialog::reset() {
    // TODO: handle error
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }
    const AdObject object = ad.search_object(target);

    for (auto tab : tabs) {
        tab->load(ad, object);
    }

    apply_button->setEnabled(false);
    reset_button->setEnabled(false);
    is_modified = false;
}

void PropertiesDialog::on_edited() {
    apply_button->setEnabled(true);
    reset_button->setEnabled(true);
    is_modified = true;
}
