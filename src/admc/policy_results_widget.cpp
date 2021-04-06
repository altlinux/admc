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

#include "policy_results_widget.h"

#include "adldap.h"
#include "console_widget/console_widget.h"
#include "central_widget.h"
#include "policy_model.h"
#include "utils.h"
#include "gplink.h"
#include "status.h"

#include <QModelIndex>
#include <QTreeView>
#include <QDebug>
#include <QTreeView>
#include <QStandardItemModel>
#include <QVBoxLayout>

enum PolicyResultsColumn {
    PolicyResultsColumn_Name,
    PolicyResultsColumn_Enforced,
    PolicyResultsColumn_Disabled,

    PolicyResultsColumn_COUNT,
};

enum PolicyResultsRole {
    PolicyResultsRole_DN = Qt::UserRole,
    PolicyResultsRole_GplinkString = Qt::UserRole + 1,

    PolicyResultsRole_COUNT,
};

const QSet<PolicyResultsColumn> option_columns = {
    PolicyResultsColumn_Disabled,
    PolicyResultsColumn_Enforced
};

const QHash<PolicyResultsColumn, GplinkOption> column_to_option = {
    {PolicyResultsColumn_Disabled, GplinkOption_Disabled},
    {PolicyResultsColumn_Enforced, GplinkOption_Enforced}
};

// TODO: need to sync this with changes done through group
// policy tab (just call load after properties is closed?)

PolicyResultsWidget::PolicyResultsWidget() {   
    view = new QTreeView(this);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);

    model = new QStandardItemModel(0, PolicyResultsColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model, {
        {PolicyResultsColumn_Name, tr("Location")},
        {PolicyResultsColumn_Enforced, tr("Enforced")},
        {PolicyResultsColumn_Disabled, tr("Disabled")},
    });

    view->setModel(model);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);

    connect(
        model, &QStandardItemModel::itemChanged,
        this, &PolicyResultsWidget::on_item_changed);
}

void PolicyResultsWidget::update(const QModelIndex &scope_index) {
    const ItemType type = (ItemType) scope_index.data(ConsoleRole_Type).toInt();
    if (type != ItemType_Policy) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    model->removeRows(0, model->rowCount());

    gpo = scope_index.data(PolicyRole_DN).toString();

    const QList<QString> search_attributes = {ATTRIBUTE_NAME, ATTRIBUTE_GPLINK};
    const QString filter = filter_CONDITION(Condition_Contains, ATTRIBUTE_GPLINK, gpo);
    const QHash<QString, AdObject> search_results = ad.search(filter, search_attributes, SearchScope_All);

    for (const AdObject &object : search_results.values()) {
        const QList<QStandardItem *> row = make_item_row(PolicyResultsColumn_COUNT);
        
        const QString dn = object.get_dn();
        const QString name = dn_get_name(dn);
        row[PolicyResultsColumn_Name]->setText(name);

        const QString gplink_string = object.get_string(ATTRIBUTE_GPLINK);
        const Gplink gplink = Gplink(gplink_string);

        const Qt::CheckState enforced_checkstate =
        [&]() {
            const bool is_enforced = gplink.get_option(dn, GplinkOption_Enforced);
            if (is_enforced) {
                return Qt::Checked;
            } else {
                return Qt::Unchecked;
            }
        }();
        row[PolicyResultsColumn_Enforced]->setCheckable(true);
        row[PolicyResultsColumn_Enforced]->setCheckState(enforced_checkstate);

        for (const auto column : option_columns) {
            QStandardItem *item = row[column];
            item->setCheckable(true);

            const Qt::CheckState checkstate =
            [=]() {
                const GplinkOption option = column_to_option[column];
                const bool option_is_set = gplink.get_option(dn, option);
                if (option_is_set) {
                    return Qt::Checked;
                } else {
                    return Qt::Unchecked;
                }
            }();
            item->setCheckState(checkstate);
        }

        row[0]->setData(dn, PolicyResultsRole_DN);
        row[0]->setData(gplink_string, PolicyResultsRole_GplinkString);

        model->appendRow(row);
    }
}

void PolicyResultsWidget::on_item_changed(QStandardItem *item) {
    const PolicyResultsColumn column = (PolicyResultsColumn) item->column();
    if (!option_columns.contains(column)) {
        return;
    }

    const QModelIndex this_index = item->index();
    const QModelIndex index = this_index.siblingAtColumn(0);
    const QString dn = index.data(PolicyResultsRole_DN).toString();
    const GplinkOption option = column_to_option[column];
    const bool is_checked = (item->checkState() == Qt::Checked);

    const QString gplink_string = index.data(PolicyResultsRole_GplinkString).toString();
    Gplink gplink = Gplink(gplink_string);
    gplink.set_option(gpo, option, is_checked);
    const QString updated_gplink_string = gplink.to_string();

    const bool gplink_didnt_change = gplink.equals(gplink_string);
    if (gplink_didnt_change) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    show_busy_indicator();

    const bool success = ad.attribute_replace_string(dn, ATTRIBUTE_GPLINK, updated_gplink_string);

    if (success) {
        model->setData(index, updated_gplink_string, PolicyResultsRole_GplinkString);
    } else {
        const Qt::CheckState undo_check_state =
        [&]() {
            if (item->checkState() == Qt::Checked) {
                return Qt::Unchecked;
            } else {
                return Qt::Checked;
            }
        }();
        item->setCheckState(undo_check_state);
    }

    STATUS()->display_ad_messages(ad, this);

    hide_busy_indicator();
}
