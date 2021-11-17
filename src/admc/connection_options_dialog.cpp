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

    setAttribute(Qt::WA_DeleteOnClose);

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

    // Load saved options
    const int port = settings_get_variant(SETTING_port).toInt();
    ui->port_spinbox->setValue(port);

    const bool sasl_nocanon = settings_get_variant(SETTING_sasl_nocanon).toBool();
    ui->canonize_check->setChecked(sasl_nocanon);

    const QString cert_strategy = settings_get_variant(SETTING_cert_strategy, CERT_STRATEGY_NEVER).toString();
    const int cert_strategy_index = ui->cert_combo->findText(cert_strategy);
    ui->cert_combo->setCurrentIndex(cert_strategy_index);

    // Populate hosts list
    const QString domain = get_default_domain_from_krb5();
    const QList<QString> host_list = get_domain_hosts(domain, QString());

    any_hosts_available = !host_list.isEmpty();

    if (any_hosts_available) {
        ui->host_warning_label->setVisible(false);

        for (const QString &host : host_list) {
            ui->host_select_list->addItem(host);
        }

        const QString saved_host = settings_get_variant(SETTING_host, QString()).toString();

        if (!saved_host.isEmpty()) {
            // Select saved host in list, if it's there.
            // Otherwise put saved host into "custom" field
            const QList<QListWidgetItem *> item_list = ui->host_select_list->findItems(saved_host, Qt::MatchExactly);
            const bool saved_host_is_in_list = !item_list.isEmpty();

            if (saved_host_is_in_list) {
                ui->host_select_button->setChecked(true);
                QListWidgetItem *item = item_list[0];
                ui->host_select_list->setCurrentItem(item);
            } else {
                ui->host_custom_button->setChecked(true);
                ui->host_custom_edit->setText(saved_host);
            }
        } else {
            // If saved host is empty, select first available
            // host in select list
            ui->host_select_button->setChecked(true);
            ui->host_select_list->setCurrentRow(0);
        }
    } else {
        ui->host_frame->setEnabled(false);
        ui->host_warning_label->setVisible(true);
    }

    settings_setup_dialog_geometry(SETTING_connection_options_dialog_geometry, this);

    connect(
        ui->button_box->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
        this, &ConnectionOptionsDialog::load_default_options);
}

ConnectionOptionsDialog::~ConnectionOptionsDialog() {
    delete ui;
}

void ConnectionOptionsDialog::accept() {
    if (any_hosts_available) {
        const bool host_is_valid = [&]() {
            const bool host_is_custom = ui->host_custom_button->isChecked();

            if (host_is_custom) {
                const QString custom_host = ui->host_custom_edit->text();

                return !custom_host.isEmpty();
            } else {
                const bool any_host_selected = !ui->host_select_list->selectedItems().isEmpty();

                return any_host_selected;
            }
        }();

        if (!host_is_valid) {
            message_box_warning(this, tr("Error"), tr("Select or enter a host."));

            return;
        }
    }

    const int port = ui->port_spinbox->value();
    settings_set_variant(SETTING_port, port);

    const bool sasl_nocanon = ui->canonize_check->isChecked();
    settings_set_variant(SETTING_sasl_nocanon, sasl_nocanon);

    const QString cert_strategy = ui->cert_combo->currentText();
    settings_set_variant(SETTING_cert_strategy, cert_strategy);

    if (any_hosts_available) {
        const QString selected_host = [&]() {
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
        }();

        settings_set_variant(SETTING_host, selected_host);
    }

    load_connection_options();

    QDialog::accept();
}

void ConnectionOptionsDialog::load_default_options() {
    ui->port_spinbox->setValue(0);

    ui->canonize_check->setChecked(true);

    const int never_index = ui->cert_combo->findText(CERT_STRATEGY_NEVER);
    ui->cert_combo->setCurrentIndex(never_index);

    if (any_hosts_available) {
        ui->host_select_button->setChecked(true);
        ui->host_select_list->setCurrentRow(0);
        ui->host_custom_edit->clear();
    }
}

void load_connection_options() {
    const QString saved_dc = settings_get_variant(SETTING_host).toString();
    AdInterface::set_dc(saved_dc);

    const QVariant sasl_nocanon = settings_get_variant(SETTING_sasl_nocanon);
    if (sasl_nocanon.isValid()) {
        AdInterface::set_sasl_nocanon(sasl_nocanon.toBool());
    } else {
        AdInterface::set_sasl_nocanon(true);
    }

    const QVariant port = settings_get_variant(SETTING_port);
    if (port.isValid()) {
        AdInterface::set_port(port.toInt());
    } else {
        AdInterface::set_port(0);
    }

    const QString cert_strategy_string = settings_get_variant(SETTING_cert_strategy).toString();
    const QHash<QString, CertStrategy> cert_strategy_map = {
        {CERT_STRATEGY_NEVER, CertStrategy_Never},
        {CERT_STRATEGY_HARD, CertStrategy_Hard},
        {CERT_STRATEGY_DEMAND, CertStrategy_Demand},
        {CERT_STRATEGY_ALLOW, CertStrategy_Allow},
        {CERT_STRATEGY_TRY, CertStrategy_Try},
    };
    const CertStrategy cert_strategy = cert_strategy_map.value(cert_strategy_string, CertStrategy_Never);
    AdInterface::set_cert_strategy(cert_strategy);
}
