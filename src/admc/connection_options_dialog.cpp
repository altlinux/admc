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

ConnectionOptionsDialog::ConnectionOptionsDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::ConnectionOptionsDialog();
    ui->setupUi(this);

    const QList<QString> require_cert_list = {
        CERT_STRATEGY_NEVER,
        "hard",
        "demand",
        "allow",
        "try",
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

void ConnectionOptionsDialog::accept() {
    const int port = ui->port_spinbox->value();
    settings_set_variant(SETTING_port, port);

    const bool sasl_nocanon = ui->canonize_check->isChecked();
    settings_set_variant(SETTING_sasl_nocanon, sasl_nocanon);

    const QString cert_strategy = ui->cert_combo->currentText();
    settings_set_variant(SETTING_cert_strategy, cert_strategy);

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

    // TODO: verify that this is indeed the default value
    const QString cert_strategy = settings_get_variant(SETTING_cert_strategy, CERT_STRATEGY_NEVER).toString();
    const int cert_strategy_index = ui->cert_combo->findText(cert_strategy);
    ui->cert_combo->setCurrentIndex(cert_strategy_index);
}

void ConnectionOptionsDialog::restore_defaults() {
    ui->port_spinbox->setValue(0);

    ui->canonize_check->setChecked(true);

    const int never_index = ui->cert_combo->findText(CERT_STRATEGY_NEVER);
    ui->cert_combo->setCurrentIndex(never_index);
}
