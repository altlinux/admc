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

#include "connection_options_dialog.h"
#include "ui_connection_options_dialog.h"

#include "adldap.h"
#include "settings.h"
#include "utils.h"

#include <QPushButton>

const QString CERT_STRATEGY_NEVER = "never";
const QString CERT_STRATEGY_HARD = "hard";
const QString CERT_STRATEGY_DEMAND = "demand";
const QString CERT_STRATEGY_ALLOW = "allow";
const QString CERT_STRATEGY_TRY = "try";

ConnectionOptionsDialog::ConnectionOptionsDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::ConnectionOptionsDialog();
    ui->setupUi(this);

    const QList<QString> require_cert_list = {
        CERT_STRATEGY_NEVER,
        CERT_STRATEGY_HARD,
        CERT_STRATEGY_DEMAND,
        CERT_STRATEGY_ALLOW,
        CERT_STRATEGY_TRY,
    };
    for (const QString &string : require_cert_list) {
        ui->cert_combo->addItem(string);
    }

    settings_setup_dialog_geometry(SETTING_connection_options_dialog_geometry, this);

    connect(
        ui->button_box->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
        this, &ConnectionOptionsDialog::restore_defaults);

    reset();
}

// Load hosts into list on open. Can't do this in ctor
// because hosts lists can change between opens.
void ConnectionOptionsDialog::open() {
    const QString prev_host = get_selected_host();

    const QString domain = get_default_domain_from_krb5();
    const QList<QString> host_list = get_domain_hosts(domain, QString());

    ui->host_select_list->clear();

    for (const QString &host : host_list) {
        ui->host_select_list->addItem(host);
    }

    // Restore previous host selection
    const QList<QListWidgetItem *> item_list = ui->host_select_list->findItems(prev_host, Qt::MatchExactly);
    if (ui->host_select_button->isChecked() && !item_list.isEmpty()) {
        QListWidgetItem *item = item_list[0];
        ui->host_select_list->setCurrentItem(item);
    }

    QDialog::open();
}

ConnectionOptionsDialog::~ConnectionOptionsDialog() {
    delete ui;
}

void ConnectionOptionsDialog::accept() {
    const bool host_is_valid = [&]() {
        if (ui->host_select_button->isChecked()) {
            // Select case. Must select host, if any are available, if
            // none available then no selection is ok
            if (ui->host_select_list->count() > 0) {
                const bool any_host_selected = !ui->host_select_list->selectedItems().isEmpty();

                return any_host_selected;
            } else {
                return true;
            }
        } else {
            // Custom case. Must enter non-empty string
            return !ui->host_custom_edit->text().isEmpty();
        }
    }();

    if (!host_is_valid) {
        message_box_warning(this, tr("Error"), tr("Select or enter a host."));

        return;
    }

    const int port = ui->port_spinbox->value();
    settings_set_variant(SETTING_port, port);

    const bool sasl_nocanon = ui->canonize_check->isChecked();
    settings_set_variant(SETTING_sasl_nocanon, sasl_nocanon);

    const QString cert_strategy = ui->cert_combo->currentText();
    settings_set_variant(SETTING_cert_strategy, cert_strategy);

    const QString selected_host = get_selected_host();
    settings_set_variant(SETTING_host, selected_host);

    QDialog::accept();
}

void ConnectionOptionsDialog::reject() {
    reset();

    QDialog::reject();
}

void ConnectionOptionsDialog::reset() {
    const int port = settings_get_variant(SETTING_port).toInt();
    ui->port_spinbox->setValue(port);

    const bool sasl_nocanon = settings_get_bool(SETTING_sasl_nocanon);
    ui->canonize_check->setChecked(sasl_nocanon);

    const QString cert_strategy = settings_get_variant(SETTING_cert_strategy, CERT_STRATEGY_NEVER).toString();
    const int cert_strategy_index = ui->cert_combo->findText(cert_strategy);
    ui->cert_combo->setCurrentIndex(cert_strategy_index);

    ui->host_select_button->setChecked(true);
    ui->host_select_list->setCurrentRow(0);
}

void ConnectionOptionsDialog::restore_defaults() {
    ui->port_spinbox->setValue(0);

    ui->canonize_check->setChecked(true);

    const int never_index = ui->cert_combo->findText(CERT_STRATEGY_NEVER);
    ui->cert_combo->setCurrentIndex(never_index);

    ui->host_select_button->setChecked(true);
    ui->host_select_list->setCurrentRow(0);
    ui->host_custom_edit->clear();
}

QString ConnectionOptionsDialog::get_selected_host() const {
    if (ui->host_select_button->isChecked()) {
        QListWidgetItem *current_item = ui->host_select_list->currentItem();

        if (current_item == nullptr) {
            return QString();
        } else {
            return current_item->text();
        }
    } else {
        return ui->host_custom_edit->text();
    }
}
