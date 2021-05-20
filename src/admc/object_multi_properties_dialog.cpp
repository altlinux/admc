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

#include "object_multi_properties_dialog.h"

#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "utils.h"
#include "tab_widget.h"
#include "multi_tabs/general_multi_tab.h"
#include "multi_tabs/account_multi_tab.h"
#include "multi_tabs/address_multi_tab.h"
#include "multi_tabs/profile_multi_tab.h"
#include "multi_tabs/organization_multi_tab.h"

#include <QAction>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDebug>
#include <QAbstractItemView>

ObjectMultiPropertiesDialog::ObjectMultiPropertiesDialog(const QList<QString> &target_list_arg, const QList<QString> &class_list)
: QDialog()
{
    target_list = target_list_arg;

    setAttribute(Qt::WA_DeleteOnClose);

    AdInterface ad;
    if (ad_failed(ad)) {
        close();
        
        return;
    }

    setWindowTitle(tr("Properties for multiple objects"));

    setMinimumHeight(700);
    setMinimumWidth(500);

    auto tab_widget = new TabWidget();

    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    apply_button = button_box->addButton(QDialogButtonBox::Apply);
    reset_button = button_box->addButton(QDialogButtonBox::Reset);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setSpacing(0);
    layout->addWidget(tab_widget);
    layout->addWidget(button_box);    

    auto add_tab =
    [&](PropertiesMultiTab *tab, const QString &title) {
        tab_widget->add_tab(tab, title);
        tab_list.append(tab);
    };

    add_tab(new GeneralMultiTab(class_list), tr("General"));

    if (class_list == QList<QString>({CLASS_USER})) {
        add_tab(new AccountMultiTab(ad), tr("Account"));
        add_tab(new AddressMultiTab(), tr("Address"));
        add_tab(new ProfileMultiTab(), tr("Profile"));
        add_tab(new OrganizationMultiTab(), tr("Organization"));
    }

    for (PropertiesMultiTab *tab : tab_list) {
        connect(
            tab, &PropertiesMultiTab::edited,
            this, &ObjectMultiPropertiesDialog::on_tab_edited);
    }

    connect(
        ok_button, &QPushButton::clicked,
        this, &ObjectMultiPropertiesDialog::ok);
    connect(
        apply_button, &QPushButton::clicked,
        this, &ObjectMultiPropertiesDialog::apply);
    connect(
        reset_button, &QPushButton::clicked,
        this, &ObjectMultiPropertiesDialog::reset);
    connect(
        cancel_button, &QPushButton::clicked,
        this, &ObjectMultiPropertiesDialog::reject);

    reset();
}

void ObjectMultiPropertiesDialog::ok() {
    const bool success = apply();

    if (success) {
        accept();
    }
}

bool ObjectMultiPropertiesDialog::apply() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return false;
    }

    show_busy_indicator();

    bool total_apply_success = true;
    for (PropertiesMultiTab *tab : tab_list) {
        const bool success = tab->apply(ad, target_list);

        if (!success) {
            total_apply_success = false;
        }
    }

    g_status()->display_ad_messages(ad, this);

    hide_busy_indicator();

    emit applied();

    return total_apply_success;
}

void ObjectMultiPropertiesDialog::reset() {
    for (PropertiesMultiTab *tab : tab_list) {
        tab->reset();
    }
}

void ObjectMultiPropertiesDialog::on_tab_edited() {
    
}
