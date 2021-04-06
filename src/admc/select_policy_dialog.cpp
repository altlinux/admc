/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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
#include "globals.h"
#include "edits/string_edit.h"
#include "status.h"
#include "utils.h"
#include "policy_model.h"

#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTreeView>
#include <QStandardItemModel>

SelectPolicyDialog::SelectPolicyDialog(QWidget *parent)
: QDialog(parent)
{
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

    model->setHorizontalHeaderLabels(policy_model_header_labels());

    const QList<QString> search_attributes = policy_model_search_attributes();
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);

    const QHash<QString, AdObject> search_results = ad.search(filter, search_attributes, SearchScope_All);

    for (const AdObject &object : search_results.values()) {
        auto item = new QStandardItem();
        model->appendRow(item);

        setup_policy_scope_item(item, object);
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
