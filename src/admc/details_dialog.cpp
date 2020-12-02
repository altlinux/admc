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
#include "tabs/organization_tab.h"
#include "tabs/telephones_tab.h"
#include "tabs/profile_tab.h"
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
#include <QAbstractItemView>

DetailsDialog *DetailsDialog::docked_instance = nullptr;
QHash<QString, DetailsDialog *> DetailsDialog::floating_instances;

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
    if (target.isEmpty()) {
        return;
    }

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
        const bool dialog_already_open_for_this_target = floating_instances.contains(target);

        if (dialog_already_open_for_this_target) {
            // Focus already open dialog
            DetailsDialog *dialog = floating_instances[target];
            dialog->raise();
            dialog->setFocus();
        } else {
            // Make new dialog for this target
            auto dialog = new DetailsDialog(target, true);

            floating_instances[target] = dialog;
            connect(
                dialog, &QDialog::finished,
                [target]() {
                    floating_instances.remove(target);
                });

            dialog->show();
        }
    }
}

void DetailsDialog::connect_to_open_by_double_click(QAbstractItemView *view, const int dn_column) {
    connect(
        view, &QAbstractItemView::doubleClicked,
        [dn_column](const QModelIndex &index) {
            const QString dn = get_dn_from_index(index, dn_column);
            open_for_target(dn);
        });
}

DetailsDialog::DetailsDialog(const QString &target_arg, const bool is_floating_instance_arg)
: QDialog()
{
    target = target_arg;
    is_floating_instance = is_floating_instance_arg;

    setAttribute(Qt::WA_DeleteOnClose);
    resize(600, 700);

    tab_widget = new QTabWidget(this);

    auto button_box = new QDialogButtonBox();
    apply_button = button_box->addButton(QDialogButtonBox::Apply);
    reset_button = button_box->addButton(QDialogButtonBox::Reset);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);

    // Make apply button "auto default", which means that
    // when a tab is edited and apply/reset get enabled,
    // apply will become focused and if user presses enter,
    // apply button will be pressed, applying the changes.
    cancel_button->setAutoDefault(false);
    reset_button->setAutoDefault(false);
    apply_button->setAutoDefault(true);

    const AdObject object = AD()->search_object(target);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setSpacing(0);

    const QString name = object.get_string(ATTRIBUTE_NAME);
    const QString window_title = name.isEmpty() ? tr("Details") : QString(tr("\"%1\" Details")).arg(name);

    if (is_floating_instance) {
        setWindowTitle(window_title);
    } else {
        // Docked instance is not a window so can't use
        // window title and have to put it in a label.
        auto title_label = new QLabel(window_title);
        layout->addWidget(title_label);
    }

    layout->addWidget(tab_widget);
    layout->addWidget(button_box);

    // TODO: is this actually possible and what should happen, currently leaving the dialog blank which might be enough.
    if (object.is_empty()) {
        return;
    }

    // Create new tabs
    const auto add_tab =
    [this](DetailsTab *tab, const QString &title) {
        tabs.append(tab);
        tab_widget->addTab(tab, title);
    };

    add_tab(new GeneralTab(object), tr("General"));
    add_tab(new ObjectTab(), tr("Object"));
    add_tab(new AttributesTab(), tr("Attributes"));
    if (object.is_class(CLASS_USER)) {
        add_tab(new AccountTab(), tr("Account"));
        add_tab(new AddressTab(), tr("Address"));
        add_tab(new OrganizationTab(), tr("Organization"));
        add_tab(new TelephonesTab(), tr("Telephones"));
        add_tab(new ProfileTab(), tr("Profile"));
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
        connect(
            tab, &DetailsTab::edited,
            this, &DetailsDialog::on_edited);
    }

    reset();

    connect(
        apply_button, &QPushButton::clicked,
        this, &DetailsDialog::apply);
    connect(
        reset_button, &QPushButton::clicked,
        this, &DetailsDialog::reset);
    connect(
        cancel_button, &QPushButton::clicked,
        this, &DetailsDialog::reject);
    connect(
        AD(), &AdInterface::modified,
        this, &DetailsDialog::reload_target);

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
            QDialog::reject();
        }
    } else {
        // Hide/show docked instance depending on docked setting
        get_docked_container()->setVisible(is_docked);
    }
}

void DetailsDialog::apply() {
    STATUS()->start_error_log();

    AD()->start_batch();
    for (auto tab : tabs) {
        if (tab_widget->indexOf(tab) != -1) {
            tab->apply(target);
        }
    }
    AD()->end_batch();

    STATUS()->end_error_log(this);

    reset();
}

void DetailsDialog::reset() {
    const AdObject object = AD()->search_object(target);
    for (auto tab : tabs) {
        tab->load(object);
    }

    apply_button->setEnabled(false);
    reset_button->setEnabled(false);
}

void DetailsDialog::on_edited() {
    apply_button->setEnabled(true);
    reset_button->setEnabled(true);
}

void DetailsDialog::reload_target() {
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
