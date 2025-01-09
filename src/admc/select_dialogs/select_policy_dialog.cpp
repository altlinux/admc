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

#include "select_policy_dialog.h"
#include "ui_select_policy_dialog.h"

#include "adldap.h"
#include "console_impls/policy_impl.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QPushButton>
#include <QStandardItemModel>

SelectPolicyDialog::SelectPolicyDialog(AdInterface &ad, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::SelectPolicyDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    model = new QStandardItemModel(this);
    ui->view->setModel(model);

    ui->view->setHeaderHidden(true);

    QPushButton *ok_button = ui->button_box->button(QDialogButtonBox::Ok);
    enable_widget_on_selection(ok_button, ui->view);

    const QString base = g_adconfig->domain_dn();
    const SearchScope scope = SearchScope_All;
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);
    const QList<QString> attributes = QList<QString>();

    const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

    for (const AdObject &object : results.values()) {
        auto item = new QStandardItem();
        model->appendRow(item);

        console_policy_load_item(item, object);
    }

    settings_setup_dialog_geometry(SETTING_select_policy_dialog_geometry, this);
}

SelectPolicyDialog::~SelectPolicyDialog() {
    delete ui;
}

QList<QString> SelectPolicyDialog::get_selected_dns() const {
    QList<QString> dns;

    const QList<QModelIndex> indexes = ui->view->selectionModel()->selectedRows();
    for (const QModelIndex &index : indexes) {
        const QString dn = index.data(PolicyRole_DN).toString();
        dns.append(dn);
    }

    return dns;
}
