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

#include "properties_multi_dialog.h"
#include "ui_properties_multi_dialog.h"

#include "adldap.h"
#include "globals.h"
#include "multi_tabs/account_multi_tab.h"
#include "multi_tabs/address_multi_tab.h"
#include "multi_tabs/general_other_multi_tab.h"
#include "multi_tabs/general_user_multi_tab.h"
#include "multi_tabs/organization_multi_tab.h"
#include "multi_tabs/profile_multi_tab.h"
#include "settings.h"
#include "status.h"
#include "tab_widget.h"
#include "utils.h"

#include <QAction>
#include <QDialogButtonBox>
#include <QPushButton>

PropertiesMultiDialog::PropertiesMultiDialog(const QList<QString> &target_list_arg, const QList<QString> &class_list)
: QDialog() {
    ui = new Ui::PropertiesMultiDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    target_list = target_list_arg;

    AdInterface ad;
    if (ad_failed(ad)) {
        close();

        return;
    }

    auto apply_button = ui->button_box->button(QDialogButtonBox::Apply);
    auto reset_button = ui->button_box->button(QDialogButtonBox::Reset);

    auto add_tab = [&](PropertiesMultiTab *tab, const QString &title) {
        ui->tab_widget->add_tab(tab, title);
        tab_list.append(tab);
    };

    if (class_list == QList<QString>({CLASS_USER})) {
        add_tab(new GeneralUserMultiTab(), tr("General"));
        add_tab(new AccountMultiTab(ad), tr("Account"));
        add_tab(new AddressMultiTab(), tr("Address"));
        add_tab(new ProfileMultiTab(), tr("Profile"));
        add_tab(new OrganizationMultiTab(), tr("Organization"));
    } else {
        add_tab(new GeneralOtherMultiTab(), tr("General"));
    }

    settings_setup_dialog_geometry(SETTING_object_multi_dialog_geometry, this);

    connect(
        apply_button, &QPushButton::clicked,
        this, &PropertiesMultiDialog::apply);
    connect(
        reset_button, &QPushButton::clicked,
        this, &PropertiesMultiDialog::reset);

    reset();
}

PropertiesMultiDialog::~PropertiesMultiDialog() {
    delete ui;
}

void PropertiesMultiDialog::accept() {
    const bool success = apply();

    if (success) {
        QDialog::accept();
    }
}

bool PropertiesMultiDialog::apply() {
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

void PropertiesMultiDialog::reset() {
    for (PropertiesMultiTab *tab : tab_list) {
        tab->reset();
    }
}
