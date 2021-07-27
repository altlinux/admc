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

#include "adldap.h"
#include "settings.h"
#include "utils.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QListWidget>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>

ConnectionOptionsDialog::ConnectionOptionsDialog(QWidget *parent)
: QDialog(parent) {
    setWindowTitle(tr("Change Domain Controller"));

    sasl_canon_check = new QCheckBox(tr("Canonize hostname"));

    port_edit = new QLineEdit();
    set_line_edit_to_numbers_only(port_edit);

    require_cert_combobox = new QComboBox();
    const QList<QString> require_cert_list = {
        "never",
        "hard",
        "demand",
        "allow",
        "try",
    };
    for (const QString &string : require_cert_list) {
        require_cert_combobox->addItem(string);
    }

    auto button_box = new QDialogButtonBox();
    button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);
    auto reset_button = button_box->addButton(QDialogButtonBox::Reset);

    auto form_layout = new QFormLayout();
    form_layout->addRow(tr("Port:"), port_edit);
    form_layout->addRow(tr("Require cert strategy:"), require_cert_combobox);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(sasl_canon_check);
    layout->addLayout(form_layout);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
    connect(
        reset_button, &QPushButton::clicked,
        this, &ConnectionOptionsDialog::reset);

    reset();
}

void ConnectionOptionsDialog::reset() {
    const QString port = settings_get_variant(SETTING_port).toString();
    port_edit->setText(port);
    
    const bool sasl_canon = settings_get_variant(SETTING_sasl_canon).toBool();
    sasl_canon_check->setChecked(sasl_canon);

    // TODO: verify that this is indeed the default value
    const QString cert_strategy = settings_get_variant(SETTING_cert_strategy, "never").toString();
    const int cert_strategy_index = require_cert_combobox->findText(cert_strategy);
    require_cert_combobox->setCurrentIndex(cert_strategy_index);
}

void ConnectionOptionsDialog::reject() {
    reset();

    QDialog::reject();
}

void ConnectionOptionsDialog::accept() {
    const QString port = port_edit->text();
    settings_set_variant(SETTING_port, port);

    const bool sasl_canon = sasl_canon_check->isChecked();
    settings_set_variant(SETTING_sasl_canon, sasl_canon);

    const QString cert_strategy = require_cert_combobox->currentText();
    settings_set_variant(SETTING_cert_strategy, cert_strategy);

    QDialog::accept();
}
