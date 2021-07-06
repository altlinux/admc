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

#include "adldap.h"
#include "change_dc_dialog.h"
#include "console_types/console_object.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QVBoxLayout>

ChangeDCDialog::ChangeDCDialog(QStandardItem *domain_head_item_arg, QWidget *parent)
: QDialog(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(500, 200);
    setWindowTitle(tr("Change domain controller"));

    domain_head_item = domain_head_item_arg;

    list_widget = new QListWidget();

    const QString domain = get_default_domain_from_krb5();
    const QList<QString> host_list = get_domain_hosts(domain, QString());

    for (const QString &host : host_list) {
        list_widget->addItem(host);
    }

    save_dc_checkbox = new QCheckBox(tr("Save this setting"));

    auto button_box = new QDialogButtonBox();
    button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(list_widget);
    layout->addWidget(save_dc_checkbox);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
}

void ChangeDCDialog::accept() {
    QListWidgetItem *current_item = list_widget->currentItem();
    const QString current_dc = current_item->text();
    AdInterface::set_dc(current_dc);

    console_object_load_domain_head_text(domain_head_item);

    if (save_dc_checkbox->isChecked()) {
        g_settings->set_variant(VariantSetting_DC, current_dc);
    }

    QDialog::accept();
}
