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
#include <QLineEdit>
#include <QListWidget>
#include <QRadioButton>
#include <QVBoxLayout>

ChangeDCDialog::ChangeDCDialog(QStandardItem *domain_head_item_arg, QWidget *parent)
: QDialog(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(500, 200);
    setWindowTitle(tr("Change Domain Controller"));

    domain_head_item = domain_head_item_arg;

    select_button = new QRadioButton(tr("Select:"));
    auto custom_button = new QRadioButton(tr("Custom:"));

    list_widget = new QListWidget();

    const QString domain = get_default_domain_from_krb5();
    const QList<QString> host_list = get_domain_hosts(domain, QString());

    for (const QString &host : host_list) {
        list_widget->addItem(host);
    }

    custom_dc_edit = new QLineEdit();

    save_dc_checkbox = new QCheckBox(tr("Save this setting"));

    auto button_box = new QDialogButtonBox();
    button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(select_button);
    layout->addWidget(list_widget);
    layout->addWidget(custom_button);
    layout->addWidget(custom_dc_edit);
    layout->addWidget(save_dc_checkbox);
    layout->addWidget(button_box);

    select_button->setChecked(true);
    custom_dc_edit->setEnabled(false);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
    connect(
        select_button, &QAbstractButton::toggled,
        list_widget, &QWidget::setEnabled);
    connect(
        custom_button, &QAbstractButton::toggled,
        custom_dc_edit, &QWidget::setEnabled);
}

void ChangeDCDialog::accept() {
    const QString selected_dc = [&]() {
        if (select_button->isChecked()) {
            QListWidgetItem *current_item = list_widget->currentItem();

            if (current_item == nullptr) {
                return QString();
            } else {
                return current_item->text();
            }
        } else {
            return custom_dc_edit->text();
        }
    }();

    if (selected_dc.isEmpty()) {
        message_box_warning(this, tr("Error"), tr("Select or enter a domain controller."));

        return;
    }
    
    AdInterface::set_dc(selected_dc);

    console_object_load_domain_head_text(domain_head_item);

    if (save_dc_checkbox->isChecked()) {
        settings_set_variant(SETTING_dc, selected_dc);
    }

    QDialog::accept();
}
