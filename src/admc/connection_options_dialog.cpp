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

#include "connection_options_dialog.h"
#include "ui_connection_options_dialog.h"

#include "adldap.h"
#include "settings.h"
#include "utils.h"
#include "fsmo/fsmo_utils.h"
#include "globals.h"
#include "status.h"

#include <QPushButton>

#include <functional>

const QString CERT_STRATEGY_NEVER = CERT_STRATEGY_NEVER_define;
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

    const QString cert_strategy = settings_get_variant(SETTING_cert_strategy).toString();
    const int cert_strategy_index = ui->cert_combo->findText(cert_strategy);
    ui->cert_combo->setCurrentIndex(cert_strategy_index);

    default_domain = get_default_domain_from_krb5();
    default_host_list = get_domain_hosts(default_domain, QString());

    custom_domain = settings_get_variant(SETTING_custom_domain).toString();
    custom_host_list = get_domain_hosts(custom_domain, QString());

    // Populate hosts list
    bool domain_is_default = settings_get_variant(SETTING_domain_is_default).toBool();
    QStringList host_list;

    QString domain;
    if (domain_is_default) {
        domain = default_domain;
        host_list = default_host_list;
        ui->host_default_button->setChecked(true);
        ui->domain_custom_edit->setEnabled(false);
        ui->domain_custom_edit->setText(default_domain);
        ui->get_hosts_button->setVisible(false);
    }
    else {
        domain = custom_domain;
        host_list = custom_host_list;
        ui->host_custom_button->setChecked(true);
        ui->domain_custom_edit->setEnabled(true);
        ui->domain_custom_edit->setText(custom_domain);
        ui->get_hosts_button->setVisible(true);
    }

    any_hosts_available = !host_list.isEmpty();

    if (any_hosts_available) {
        ui->host_warning_label->setVisible(false);

        for (const QString &host : host_list) {
            ui->host_select_list->addItem(host);
        }

        set_saved_host_current_item();
    } else {
        ui->host_warning_label->setVisible(true);
    }

    settings_setup_dialog_geometry(SETTING_connection_options_dialog_geometry, this);

    connect(ui->button_box->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
        this, &ConnectionOptionsDialog::load_default_options);

    connect(ui->host_default_button, &QRadioButton::toggled,
            this, &ConnectionOptionsDialog::host_button_toggled);

    connect(ui->get_hosts_button, &QPushButton::clicked,
            this, &ConnectionOptionsDialog::get_hosts);
}

ConnectionOptionsDialog::~ConnectionOptionsDialog() {
    delete ui;
}

void ConnectionOptionsDialog::accept() {
    if (any_hosts_available) {
        const bool any_host_selected = !ui->host_select_list->selectedItems().isEmpty();

        if (!any_host_selected) {
            message_box_warning(this, tr("Error"), tr("Select a host."));
            return;
        }
    }

    const int port = ui->port_spinbox->value();

    const bool sasl_nocanon = ui->canonize_check->isChecked();

    const QString cert_strategy = ui->cert_combo->currentText();

    QString selected_host;
    QListWidgetItem *item = ui->host_select_list->currentItem();
    if (item == nullptr) {
        selected_host = QString();
    }
    else {
        selected_host = item->text();
    }

    const bool domain_was_default = settings_get_variant(SETTING_domain_is_default).toBool();
    const bool domain_is_default = ui->host_default_button->isChecked();
    const bool custom_domain_changed = settings_get_variant(SETTING_custom_domain).toString() != custom_domain;

    const QHash<QString, QVariant> settings = {
        {SETTING_port, port},
        {SETTING_sasl_nocanon, sasl_nocanon},
        {SETTING_cert_strategy, cert_strategy},
        {SETTING_host, selected_host},
        {SETTING_custom_domain, custom_domain},
        {SETTING_domain_is_default, domain_is_default}
    };

    load_connection_options(settings);

    AdInterface ad;
    if (ad_failed(ad, parentWidget())) {
        // load back options from config
        load_connection_options();
        return;
    }

    for (const auto& setting : settings.keys()) {
        settings_set_variant(setting, settings.value(setting));
    }

    const bool domain_is_changed = (domain_was_default != domain_is_default) || custom_domain_changed;
    if (domain_is_changed) {
        load_g_adconfig(ad);
    }

    emit host_changed(selected_host);

    if (!current_dc_is_master_for_role(ad, FSMORole_PDCEmulation)) {
        if (gpo_edit_without_PDC_disabled)
            g_status->add_message(tr("You are connected to DC without PDC-Emulator role. "
                                     "Group policy editing is prohibited by the setting."), StatusType_Success);
        else
            g_status->add_message(tr("You are connected to DC without PDC-Emulator role. "
                                     "Group policy editing is available."), StatusType_Success);
    }

    QDialog::accept();
}

void ConnectionOptionsDialog::load_default_options() {
    ui->port_spinbox->setValue(0);

    ui->canonize_check->setChecked(true);

    const int never_index = ui->cert_combo->findText(CERT_STRATEGY_NEVER);
    ui->cert_combo->setCurrentIndex(never_index);

    if (any_hosts_available) {
        ui->host_default_button->setChecked(true);
        ui->host_select_list->setCurrentRow(0);
    }
}

void ConnectionOptionsDialog::set_saved_host_current_item()
{
    const QString saved_host = settings_get_variant(SETTING_host).toString();

    if (!saved_host.isEmpty()) {
        const QList<QListWidgetItem *> item_list = ui->host_select_list->findItems(saved_host, Qt::MatchExactly);
        const bool saved_host_is_in_list = !item_list.isEmpty();

        if (saved_host_is_in_list) {
            QListWidgetItem *item = item_list[0];
            ui->host_select_list->setCurrentItem(item);
        }
    } else {
        // If saved host is empty, select first available
        // host in select list
        ui->host_select_list->setCurrentRow(0);
    }
}

void ConnectionOptionsDialog::host_button_toggled(bool is_default_checked) {
    ui->host_select_list->clear();
    ui->host_warning_label->setVisible(false);
    if (is_default_checked) {
        ui->domain_custom_edit->setEnabled(false);
        ui->domain_custom_edit->setText(default_domain);
        ui->domain_custom_edit->setPlaceholderText(QString());
        ui->get_hosts_button->setVisible(false);

        if (default_host_list.isEmpty()) {
            ui->host_warning_label->setVisible(true);
            return;
        }
        else
            ui->host_select_list->addItems(default_host_list);
    }
    else {
        ui->domain_custom_edit->setEnabled(true);
        ui->domain_custom_edit->setText(custom_domain);
        ui->domain_custom_edit->setPlaceholderText("CUSTOM.DOMAIN.COM");
        ui->get_hosts_button->setVisible(true);

        if (custom_host_list.isEmpty()) {
            ui->host_warning_label->setVisible(true);
            return;
        }
        else
            ui->host_select_list->addItems(custom_host_list);
    }
    set_saved_host_current_item();
}

void ConnectionOptionsDialog::get_hosts() {
    ui->host_select_list->clear();
    QString domain = ui->domain_custom_edit->text();
    QStringList hosts = get_domain_hosts(domain, QString());
    if (hosts.isEmpty()) {
        ui->host_warning_label->setVisible(true);
    }
    else {
        ui->host_warning_label->setVisible(false);
        custom_domain = domain;
        ui->host_select_list->addItems(hosts);
        custom_host_list = hosts;
        ui->host_select_list->setCurrentRow(0);
    }
}

void load_connection_options(const QHash<QString, QVariant> &settings) {
    std::function<QVariant(const QString &setting)> get_setting;
    if (settings.isEmpty()) {
        get_setting = [](const QString &setting) {
            QVariant out = settings_get_variant(setting);
            return out;
        };
    }
    else {
        get_setting = [settings](const QString &setting) {
            QVariant out = settings[setting];
            return out;
        };
    }

    bool domain_is_default = true;
    if (get_setting(SETTING_domain_is_default).isValid())
        domain_is_default = get_setting(SETTING_domain_is_default).toBool();

    AdInterface::set_domain_is_default(domain_is_default);

    const QString custom_domain = get_setting(SETTING_custom_domain).toString();
    AdInterface::set_custom_domain(custom_domain);

    const QString saved_dc = get_setting(SETTING_host).toString();
    AdInterface::set_dc(saved_dc);

    const QVariant sasl_nocanon = get_setting(SETTING_sasl_nocanon);
    if (sasl_nocanon.isValid()) {
        AdInterface::set_sasl_nocanon(sasl_nocanon.toBool());
    } else {
        AdInterface::set_sasl_nocanon(true);
    }

    const QVariant port = get_setting(SETTING_port);
    if (port.isValid()) {
        AdInterface::set_port(port.toInt());
    } else {
        AdInterface::set_port(0);
    }

    const QString cert_strategy_string = get_setting(SETTING_cert_strategy).toString();
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
