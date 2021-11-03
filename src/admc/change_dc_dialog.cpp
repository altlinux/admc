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

#include "change_dc_dialog.h"
#include "ui_change_dc_dialog.h"

#include "adldap.h"
#include "settings.h"
#include "utils.h"
#include "widget_state.h"

ChangeDCDialog::ChangeDCDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::ChangeDCDialog();
    ui->setupUi(this);

    const QList<QWidget *> widget_list = {
        ui->select_button,
        ui->select_listwidget,
        ui->custom_button,
        ui->custom_edit,
        ui->save_this_setting_check,
    };

    state.set_widget_list(widget_list);

    settings_setup_dialog_geometry(SETTING_change_dc_dialog_geometry, this);
}

ChangeDCDialog::~ChangeDCDialog() {
    delete ui;
}

void ChangeDCDialog::open() {
    // Load hosts into list when dialog is opened for the
    // first time
    if (ui->select_listwidget->count() == 0) {
        const QString domain = get_default_domain_from_krb5();
        const QList<QString> host_list = get_domain_hosts(domain, QString());

        for (const QString &host : host_list) {
            ui->select_listwidget->addItem(host);
        }
    }

    state.save();

    QDialog::open();
}

void ChangeDCDialog::accept() {
    const QString selected_dc = [&]() {
        if (ui->select_button->isChecked()) {
            QListWidgetItem *current_item = ui->select_listwidget->currentItem();

            if (current_item == nullptr) {
                return QString();
            } else {
                return current_item->text();
            }
        } else {
            return ui->custom_edit->text();
        }
    }();

    const bool dc_is_valid = !selected_dc.isEmpty();

    if (dc_is_valid) {
        AdInterface::set_dc(selected_dc);

        if (ui->save_this_setting_check->isChecked()) {
            settings_set_variant(SETTING_dc, selected_dc);
        }

        QDialog::accept();
    } else {
        message_box_warning(this, tr("Error"), tr("Select or enter a domain controller."));
    }
}

void ChangeDCDialog::reject() {
    state.restore();

    QDialog::reject();
}
