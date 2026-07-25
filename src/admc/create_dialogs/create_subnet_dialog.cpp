/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2025-2026 BaseALT Ltd.
 * Copyright (C) 2025 Semyon Knyazev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include "create_subnet_dialog.h"
#include "ui_create_subnet_dialog.h"
#include <arpa/inet.h>
#include "ad_interface.h"
#include "ad_filter.h"
#include "ad_object.h"
#include "ad_config.h"
#include "globals.h"
#include <QPushButton>
#include "utils.h"
#include "ad_utils.h"
#include "status.h"

CreateSubnetDialog::CreateSubnetDialog(AdInterface &ad, const QString &parent_dn_arg, QWidget *parent) :
    CreateObjectDialog(parent),
    ui(new Ui::CreateSubnetDialog), parent_dn(parent_dn_arg) {
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    QRegularExpression regex(
        "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)/"
        "([0-9]|[12][0-9]|3[0-2])$"
        "|"
        "^([0-9a-fA-F]{0,4}:){2,7}[0-9a-fA-F]{0,4}/"
        "([0-9]|[1-9][0-9]|1[01][0-9]|12[0-8])$"
    );
    ui->prefix_edit->setValidator(new QRegularExpressionValidator(regex, this));
    connect(ui->prefix_edit, &QLineEdit::textEdited, this, &CreateSubnetDialog::check_prefix_validity);

    const QString sites_container_dn = "CN=Sites,CN=Configuration," + g_adconfig->root_domain_dn();
    const QString site_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_SITE);
    const QHash<QString, AdObject> site_objects = ad.search(sites_container_dn, SearchScope_Children,
                                                            site_filter, {ATTRIBUTE_DN, ATTRIBUTE_NAME});
    for (auto dn : site_objects.keys()) {
        ui->sites_cmbbox->addItem(site_objects[dn].get_string(ATTRIBUTE_NAME), dn);
    }
}

CreateSubnetDialog::~CreateSubnetDialog() {
    delete ui;
}

void CreateSubnetDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    const QString name = ui->prefix_edit->text();
    const QString dn = dn_from_name_and_parent(name, parent_dn, CLASS_SUBNET);

    const QString site_obj_dn = ui->sites_cmbbox->currentData().toString();
    const QHash<QString, QList<QString>> attr_map = {
        {ATTRIBUTE_OBJECT_CLASS, {CLASS_SUBNET}},
        {ATTRIBUTE_SITE_OBJECT, {site_obj_dn}}
    };

    const bool add_success = ad.object_add(dn, attr_map);
    g_status->display_ad_messages(ad, this);
    if (!add_success) {
        g_status->add_message(tr("Failed to create subnet object %1").arg(name), StatusType_Error);
        return;
    }

    created_dn = dn;
    g_status->add_message(tr("Subnet object %1 has been successfully created.").arg(name), StatusType_Success);
    QDialog::accept();
}

QString CreateSubnetDialog::get_created_dn() const {
    return created_dn;
}

void CreateSubnetDialog::check_prefix_validity(const QString &address) {
    // Split into address and prefix
    QStringList ip_parts = address.split('/');
    if (ip_parts.size() != 2) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
        ui->ad_name_edit->clear();
        return;
    }

    if (address.contains('.')) {
        QByteArray ipv4_bytes = ip_parts[0].toUtf8();
        quint32 ipv4;

        // Parse IP str without prefix part to int
        if (inet_pton(AF_INET, ipv4_bytes.constData(), &ipv4) != 1) {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
            ui->ad_name_edit->clear();
            return;
        }

        int ip_prefix = ip_parts[1].toInt();
        if (!validate_ipv4_prefix(ipv4, ip_prefix)) {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
            ui->ad_name_edit->clear();
            return;
        }

    } else {
        QByteArray ipv6_bytes = ip_parts[0].toUtf8();
        quint8 ipv6[16];

        // Parse IP str without prefix part to 16 byte buffer
        if (inet_pton(AF_INET6, ipv6_bytes.constData(), &ipv6) != 1) {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
            ui->ad_name_edit->clear();
            return;
        }

        int ip_prefix = ip_parts[1].toInt();
        if (!validate_ipv6_prefix(ipv6, ip_prefix)) {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
            ui->ad_name_edit->clear();
            return;
        }
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
    ui->ad_name_edit->setText(ui->prefix_edit->text());
}

bool CreateSubnetDialog::validate_ipv4_prefix(quint32 ip, int prefix) {
    if (prefix < 0 || prefix > 32) {
        return false;
    }

    if (prefix == 0) {
        return (ip == 0);
    }

    if (prefix == 32) {
        return true;
    }

    quint32 ip_host = ntohl(ip);

    quint32 mask = 0xFFFFFFFFu << (32 - prefix);

    quint32 network = ip_host & mask;

    return (ip_host == network);
}

bool CreateSubnetDialog::validate_ipv6_prefix(const quint8 ipv6[], int prefix) {
    if (prefix < 0 || prefix > 128) {
        return false;
    }

    if (prefix == 0) {
        for (int i = 0; i < 16; i++) {
            if (ipv6[i] != 0) {
                return false;
            }
        }
        return true;
    }

    if (prefix == 128) {
        return true;
    }

    int full_bytes = prefix / 8;
    int remaining_bits = prefix % 8;

    int start_check = full_bytes;
    if (remaining_bits > 0) {
        start_check++;
    }

    for (int i = start_check; i < 16; i++) {
        if (ipv6[i] != 0) {
            return false;
        }
    }

    if (remaining_bits > 0) {
        uint8_t host_mask = (1 << (8 - remaining_bits)) - 1;

        if ((ipv6[full_bytes] & host_mask) != 0) {
            return false;
        }
    }

    return true;
}


