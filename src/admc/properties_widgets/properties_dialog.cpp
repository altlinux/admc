/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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
#include "ui_properties_dialog.h"

#include "adldap.h"
#include "console_impls/object_impl.h"
#include "globals.h"
#include "properties_warning_dialog.h"
#include "security_sort_warning_dialog.h"
#include "settings.h"
#include "status.h"
#include "tab_widget.h"
#include "tabs/account_tab.h"
#include "tabs/address_tab.h"
#include "tabs/attributes_tab.h"
#include "tabs/delegation_tab.h"
#include "tabs/error_tab.h"
#include "tabs/general_computer_tab.h"
#include "tabs/general_group_tab.h"
#include "tabs/general_other_tab.h"
#include "tabs/general_ou_tab.h"
#include "tabs/general_policy_tab.h"
#include "tabs/general_shared_folder_tab.h"
#include "tabs/general_user_tab.h"
#include "tabs/group_policy_tab.h"
#include "tabs/laps_tab.h"
#include "tabs/managed_by_tab.h"
#include "tabs/membership_tab.h"
#include "tabs/object_tab.h"
#include "tabs/organization_tab.h"
#include "tabs/os_tab.h"
#include "tabs/profile_tab.h"
#include "tabs/security_tab.h"
#include "tabs/telephones_tab.h"
#include "utils.h"

#include <QAbstractItemView>
#include <QAction>
#include <QDebug>
#include <QLabel>
#include <QPushButton>

QHash<QString, PropertiesDialog *> PropertiesDialog::instances;

PropertiesDialog *PropertiesDialog::open_for_target(AdInterface &ad, const QString &target, bool *dialog_is_new, ConsoleWidget *console) {
    if (target.isEmpty()) {
        return nullptr;
    }

    show_busy_indicator();

    const bool dialog_already_open_for_this_target = PropertiesDialog::instances.contains(target);

    PropertiesDialog *dialog;

    if (dialog_already_open_for_this_target) {
        // Focus already open dialog
        dialog = PropertiesDialog::instances[target];
        dialog->raise();
        dialog->setFocus();
    } else {
        // Make new dialog for this target
        dialog = new PropertiesDialog(ad, target, console);
        dialog->open();
    }

    hide_busy_indicator();

    if (dialog_is_new != nullptr) {
        *dialog_is_new = !dialog_already_open_for_this_target;
    }

    return dialog;
}

void PropertiesDialog::open_when_view_item_activated(QAbstractItemView *view, const int dn_role) {
    connect(
        view, &QAbstractItemView::doubleClicked,
        view,
        [view, dn_role](const QModelIndex &index) {
            AdInterface ad;
            if (ad_failed(ad, view)) {
                return;
            }

            const QString dn = index.data(dn_role).toString();

            open_for_target(ad, dn);
        });
}

PropertiesDialog::PropertiesDialog(AdInterface &ad, const QString &target_arg, ConsoleWidget *console)
: QDialog() {
    ui = new Ui::PropertiesDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    target = target_arg;

    security_warning_was_rejected = false;
    security_tab = nullptr;

    ui->tab_widget->enable_auto_switch_tab(false);

    PropertiesDialog::instances[target] = this;

    apply_button = ui->button_box->button(QDialogButtonBox::Apply);
    reset_button = ui->button_box->button(QDialogButtonBox::Reset);
    auto cancel_button = ui->button_box->button(QDialogButtonBox::Cancel);

    const QString title = [&]() {
        const QString target_name = dn_get_name(target_arg);

        if (!target_name.isEmpty()) {
            return QString(tr("%1 Properties")).arg(target_name);
        } else {
            return tr("Properties");
        }
    }();
    setWindowTitle(title);

    const AdObject object = ad.search_object(target);

    const bool is_person = (object.is_class(CLASS_USER) || object.is_class(CLASS_INET_ORG_PERSON));

    //
    // Create tabs
    //
    QWidget *general_tab = [&]() -> QWidget * {
        if (is_person || object.is_class(CLASS_CONTACT)) {
            return new GeneralUserTab(&edit_list, this);
        } else if (object.is_class(CLASS_GROUP)) {
            return new GeneralGroupTab(&edit_list, this);
        } else if (object.is_class(CLASS_OU)) {
            return new GeneralOUTab(&edit_list, this);
        } else if (object.is_class(CLASS_COMPUTER)) {
            return new GeneralComputerTab(&edit_list, this);
        } else if (object.is_class(CLASS_GP_CONTAINER)) {
            return new GeneralPolicyTab(&edit_list, this);
        } else if (object.is_class(CLASS_SHARED_FOLDER)) {
            return new GeneralSharedFolderTab(&edit_list, this);
        } else if (!object.is_empty()) {
            return new GeneralOtherTab(&edit_list, this);
        } else {
            return new ErrorTab(this);
        }
    }();

    ui->tab_widget->add_tab(general_tab, tr("General"));

    const bool advanced_view_ON = settings_get_variant(SETTING_advanced_features).toBool();

    if (advanced_view_ON && !object.is_empty()) {
        auto object_tab = new ObjectTab(&edit_list, this);
        attributes_tab = new AttributesTab(&edit_list, this);

        ui->tab_widget->add_tab(object_tab, tr("Object"));
        ui->tab_widget->add_tab(attributes_tab, tr("Attributes"));
    } else {
        attributes_tab = nullptr;
    }

    if (is_person || object.is_class(CLASS_CONTACT)) {
        auto address_tab = new AddressTab(&edit_list, this);
        auto organization_tab = new OrganizationTab(&edit_list, this);
        auto telephones_tab = new TelephonesTab(&edit_list, this);

        ui->tab_widget->add_tab(address_tab, tr("Address"));
        ui->tab_widget->add_tab(organization_tab, tr("Organization"));
        ui->tab_widget->add_tab(telephones_tab, tr("Telephones"));
    }

    if (is_person) {
        auto account_tab = new AccountTab(ad, &edit_list, this);

        ui->tab_widget->add_tab(account_tab, tr("Account"));

        const bool profile_tab_enabled = settings_get_variant(SETTING_feature_profile_tab).toBool();
        if (profile_tab_enabled) {
            auto profile_tab = new ProfileTab(&edit_list, this);
            ui->tab_widget->add_tab(profile_tab, tr("Profile"));
        }
    }

    if (object.is_class(CLASS_GROUP)) {
        auto members_tab = new MembershipTab(&edit_list, MembershipTabType_Members, this);
        ui->tab_widget->add_tab(members_tab, tr("Members"));
    }

    if (is_person || object.is_class(CLASS_COMPUTER) || object.is_class(CLASS_CONTACT) || object.is_class(CLASS_GROUP)) {
        auto member_of_tab = new MembershipTab(&edit_list, MembershipTabType_MemberOf, this);
        ui->tab_widget->add_tab(member_of_tab, tr("Member of"));
    }

    if (is_person || object.is_class(CLASS_COMPUTER)) {
        auto delegation_tab = new DelegationTab(&edit_list, this);
        ui->tab_widget->add_tab(delegation_tab, tr("Delegation"));
    }

    if (object.is_class(CLASS_OU) || object.is_class(CLASS_COMPUTER) || object.is_class(CLASS_SHARED_FOLDER)) {
        auto managed_by_tab = new ManagedByTab(&edit_list, this);
        ui->tab_widget->add_tab(managed_by_tab, tr("Managed by"));
    }

    if (object.is_class(CLASS_OU) || object.is_class(CLASS_DOMAIN)) {
        auto group_policy_tab = new GroupPolicyTab(&edit_list, console, target, this);
        ui->tab_widget->add_tab(group_policy_tab, tr("Group policy"));
    }

    if (object.is_class(CLASS_COMPUTER)) {
        auto os_tab = new OSTab(&edit_list, this);

        ui->tab_widget->add_tab(os_tab, tr("Operating System"));

        const bool laps_enabled = [&]() {
            const QList<QString> attribute_list = object.attributes();
            const bool out = (attribute_list.contains(ATTRIBUTE_LAPS_PASSWORD) && attribute_list.contains(ATTRIBUTE_LAPS_EXPIRATION));

            return out;
        }();

        if (laps_enabled) {
            auto laps_tab = new LAPSTab(&edit_list, this);
            ui->tab_widget->add_tab(laps_tab, tr("LAPS"));
        }
    }

    const bool need_security_tab = object.attributes().contains(ATTRIBUTE_SECURITY_DESCRIPTOR);
    if (need_security_tab && advanced_view_ON) {
        security_tab = new SecurityTab(&edit_list, this);
        ui->tab_widget->add_tab(security_tab, tr("Security"));
    }

    for (AttributeEdit *edit : edit_list) {
        connect(
            edit, &AttributeEdit::edited,
            [this, edit]() {
                const bool already_added = apply_list.contains(edit);

                if (!already_added) {
                    apply_list.append(edit);
                }

                apply_button->setEnabled(true);
                reset_button->setEnabled(true);
            });
    }

    reset_internal(ad, object);

    settings_setup_dialog_geometry(SETTING_properties_dialog_geometry, this);

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
        ui->tab_widget, &TabWidget::current_changed,
        this, &PropertiesDialog::on_current_tab_changed);
}

PropertiesDialog::~PropertiesDialog() {
    delete ui;
}

// NOTE: order here is important. Attributes warning
// can apply/discard changes, while security warning
// can add a modification to security tab. Therefore
// attributes warning needs to go first so that it
// won't discard security warning modification.
void PropertiesDialog::on_current_tab_changed(const int prev, const int current) {
    QWidget *prev_tab = ui->tab_widget->get_tab(prev);
    QWidget *new_tab = ui->tab_widget->get_tab(current);

    const bool switching_to_or_from_attributes = (prev_tab == attributes_tab || new_tab == attributes_tab);
    const bool is_modified = !apply_list.isEmpty();
    const bool need_attributes_warning = (switching_to_or_from_attributes && is_modified);
    if (!need_attributes_warning) {
        ui->tab_widget->set_current_tab(current);

        open_security_warning();

        return;
    }

    const PropertiesWarningType warning_type = [&]() {
        if (new_tab == attributes_tab) {
            return PropertiesWarningType_SwitchToAttributes;
        } else {
            return PropertiesWarningType_SwitchFromAttributes;
        }
    }();

    auto attributes_warning_dialog = new PropertiesWarningDialog(warning_type, this);
    attributes_warning_dialog->open();

    connect(
        attributes_warning_dialog, &PropertiesWarningDialog::applied,
        this,
        [this, current]() {
            AdInterface ad;
            if (ad_failed(ad, this)) {
                return;
            }

            apply_internal(ad);

            // NOTE: have to reset for attributes tab and other tabs
            // to load updates
            const AdObject object = ad.search_object(target);
            reset_internal(ad, object);

            ui->tab_widget->set_current_tab(current);
        });

    connect(
        attributes_warning_dialog, &PropertiesWarningDialog::discarded,
        [this, current]() {
            reset();

            ui->tab_widget->set_current_tab(current);
        });

    connect(
        attributes_warning_dialog, &PropertiesWarningDialog::rejected,
        [this, prev]() {
            ui->tab_widget->set_current_tab(prev);
        });

    // Open security warning after attributes warning
    // is finished
    connect(
        attributes_warning_dialog, &QDialog::finished,
        this, &PropertiesDialog::open_security_warning);
}

void PropertiesDialog::open_security_warning() {
    const bool security_tab_exists = (security_tab != nullptr);
    if (!security_tab_exists) {
        return;
    }

    // NOTE: if security warning was rejected once,
    // then security tab is forever read only and we
    // don't show the warning again (for this dialog
    // instance).
    if (security_warning_was_rejected) {
        return;
    }

    const bool switched_to_security_tab = [&]() {
        const QWidget *current_tab = ui->tab_widget->get_current_tab();
        const bool out = (current_tab == security_tab);

        return out;
    }();
    if (!switched_to_security_tab) {
        return;
    }

    const bool order_is_correct = security_tab->verify_acl_order();
    if (order_is_correct) {
        return;
    }

    auto security_warning_dialog = new SecuritySortWarningDialog(this);
    security_warning_dialog->open();

    connect(
        security_warning_dialog, &QDialog::accepted,
        this, &PropertiesDialog::on_security_warning_accepted);
    connect(
        security_warning_dialog, &QDialog::rejected,
        this, &PropertiesDialog::on_security_warning_rejected);
}

void PropertiesDialog::on_security_warning_accepted() {
    security_tab->fix_acl_order();
}

void PropertiesDialog::on_security_warning_rejected() {
    security_tab->set_read_only();

    security_warning_was_rejected = true;
}

void PropertiesDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    const bool success = apply_internal(ad);

    if (success) {
        QDialog::accept();
    }
}

void PropertiesDialog::done(int r) {
    PropertiesDialog::instances.remove(target);

    QDialog::done(r);
}

void PropertiesDialog::apply() {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    const bool apply_success = apply_internal(ad);

    ad.clear_messages();

    if (apply_success) {
        const AdObject object = ad.search_object(target);
        reset_internal(ad, object);
    }
}

void PropertiesDialog::reset() {
    AdInterface ad;
    if (ad_connected(ad, this)) {
        const AdObject object = ad.search_object(target);
        reset_internal(ad, object);
    }
}

bool PropertiesDialog::apply_internal(AdInterface &ad) {
    // NOTE: only verify and apply edits in the "apply
    // list", aka the edits that were edited
    const bool edits_verify_success = AttributeEdit::verify(apply_list, ad, target);

    if (!edits_verify_success) {
        return false;
    }

    show_busy_indicator();

    bool total_apply_success = true;

    const bool edits_apply_success = AttributeEdit::apply(apply_list, ad, target);
    if (!edits_apply_success) {
        total_apply_success = false;
    }

    g_status->display_ad_messages(ad, this);

    if (total_apply_success) {
        apply_button->setEnabled(false);
        reset_button->setEnabled(false);
    }

    hide_busy_indicator();

    emit applied();

    return total_apply_success;
}

void PropertiesDialog::reset_internal(AdInterface &ad, const AdObject &object) {
    AttributeEdit::load(edit_list, ad, object);

    apply_button->setEnabled(false);
    reset_button->setEnabled(false);
    apply_list.clear();

    g_status->display_ad_messages(ad, this);
}
