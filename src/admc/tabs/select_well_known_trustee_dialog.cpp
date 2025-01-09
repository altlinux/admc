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

#include "select_well_known_trustee_dialog.h"
#include "ui_select_well_known_trustee_dialog.h"

#include "ad_security.h"
#include "ad_utils.h"
#include "settings.h"
#include "utils.h"

#include <QPushButton>

SelectWellKnownTrusteeDialog::SelectWellKnownTrusteeDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::SelectWellKnownTrusteeDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    for (const QString &sid_string : well_known_sid_list) {
        auto item = new QListWidgetItem();

        const QByteArray sid_bytes = sid_string_to_bytes(sid_string);
        item->setData(Qt::UserRole, sid_bytes);

        const QString name = ad_security_get_well_known_trustee_name(sid_bytes);
        item->setText(name);

        ui->list->addItem(item);
    }

    QPushButton *ok_button = ui->button_box->button(QDialogButtonBox::Ok);
    enable_widget_on_selection(ok_button, ui->list);

    settings_setup_dialog_geometry(SETTING_select_well_known_trustee_dialog_geometry, this);
}

SelectWellKnownTrusteeDialog::~SelectWellKnownTrusteeDialog() {
    delete ui;
}

QList<QByteArray> SelectWellKnownTrusteeDialog::get_selected() const {
    QList<QByteArray> out;

    const QList<QListWidgetItem *> selected = ui->list->selectedItems();
    for (QListWidgetItem *item : selected) {
        const QByteArray sid = item->data(Qt::UserRole).toByteArray();
        out.append(sid);
    }

    return out;
}
