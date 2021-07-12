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

#include "select_policy_dialog.h"

#include "adldap.h"
#include "console_types/console_policy.h"
#include "globals.h"
#include "status.h"
#include "utils.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

SelectPolicyDialog::SelectPolicyDialog(QWidget *parent)
: QDialog(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumWidth(400);
    setMinimumHeight(500);

    AdInterface ad;
    if (ad_failed(ad)) {
        close();
        return;
    }

    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);
    connect(
        ok_button, &QPushButton::clicked,
        this, &QDialog::accept);
    connect(
        cancel_button, &QPushButton::clicked,
        this, &SelectPolicyDialog::reject);

    view = new QTreeView();
    auto model = new QStandardItemModel(this);
    view->setModel(model);

    model->setHorizontalHeaderLabels(console_policy_header_labels());

    const QString base = g_adconfig->domain_head();
    const SearchScope scope = SearchScope_All;
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);
    const QList<QString> attributes = console_policy_search_attributes();

    const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

    // TODO: assuming that policy row has 1 column, need to
    // make it depend on some constant
    for (const AdObject &object : results.values()) {
        const QList<QStandardItem *> row = {new QStandardItem()};
        model->appendRow(row);

        console_policy_load(row, object);
    }

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(view);
    top_layout->addWidget(button_box);

    enable_widget_on_selection(ok_button, view);
}

QList<QString> SelectPolicyDialog::get_selected_dns() const {
    QList<QString> dns;

    const QList<QModelIndex> indexes = view->selectionModel()->selectedRows();
    for (const QModelIndex &index : indexes) {
        const QString dn = index.data(PolicyRole_DN).toString();
        dns.append(dn);
    }

    return dns;
}
