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

#include "properties_multi_dialog.h"
#include "ui_properties_multi_dialog.h"

#include "adldap.h"
#include "attribute_edits/attribute_edit.h"
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
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>

PropertiesMultiDialog::PropertiesMultiDialog(AdInterface &ad, const QList<QString> &target_list_arg, const QList<QString> &class_list)
: QDialog() {
    ui = new Ui::PropertiesMultiDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    target_list = target_list_arg;

    apply_button = ui->button_box->button(QDialogButtonBox::Apply);

    if (class_list == QList<QString>({CLASS_USER})) {
        auto general_user_tab = new GeneralUserMultiTab(&edit_list, &check_map, this);
        auto account_tab = new AccountMultiTab(ad, &edit_list, &check_map, this);
        auto address_tab = new AddressMultiTab(&edit_list, &check_map, this);
        auto profile_tab = new ProfileMultiTab(&edit_list, &check_map, this);
        auto organization_tab = new OrganizationMultiTab(&edit_list, &check_map, this);

        ui->tab_widget->add_tab(general_user_tab, tr("General"));
        ui->tab_widget->add_tab(account_tab, tr("Account"));
        ui->tab_widget->add_tab(address_tab, tr("Address"));
        ui->tab_widget->add_tab(profile_tab, tr("Profile"));
        ui->tab_widget->add_tab(organization_tab, tr("Organization"));
    } else {
        auto general_other_tab = new GeneralOtherMultiTab(&edit_list, &check_map, this);

        ui->tab_widget->add_tab(general_other_tab, tr("General"));
    }

    for (AttributeEdit *edit : check_map.keys()) {
        QCheckBox *apply_check = check_map[edit];

        connect(
            apply_check, &QAbstractButton::toggled,
            this, &PropertiesMultiDialog::on_edited);
        connect(
            apply_check, &QAbstractButton::toggled,
            edit,
            [edit, apply_check]() {
                const bool enabled = apply_check->isChecked();
                edit->set_enabled(enabled);
            });
    }

    for (AttributeEdit *edit : edit_list) {
        edit->set_enabled(false);
    }

    settings_setup_dialog_geometry(SETTING_object_multi_dialog_geometry, this);

    connect(
        apply_button, &QPushButton::clicked,
        this, &PropertiesMultiDialog::apply);
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
    if (ad_failed(ad, this)) {
        return false;
    }

    show_busy_indicator();

    const bool apply_success = [&]() {
        bool out = true;

        for (AttributeEdit *edit : edit_list) {
            QCheckBox *apply_check = check_map[edit];
            const bool need_to_apply = apply_check->isChecked();

            if (need_to_apply) {
                const bool success = [&]() {
                    bool success_out = true;

                    for (const QString &target : target_list) {
                        const bool this_success = edit->apply(ad, target);

                        success_out = (success_out && this_success);
                    }

                    return success_out;
                }();

                if (success) {
                    apply_check->setChecked(false);
                }

                out = (out && success);
            }
        }

        return out;
    }();

    g_status->display_ad_messages(ad, this);

    hide_busy_indicator();

    emit applied();

    return apply_success;
}

void PropertiesMultiDialog::on_edited() {
    apply_button->setEnabled(true);
}
