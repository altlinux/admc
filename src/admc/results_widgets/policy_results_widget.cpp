/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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
#include "ui_policy_results_widget.h"

#include "adldap.h"
#include "console_impls/item_type.h"
#include "console_impls/policy_impl.h"
#include "console_widget/console_widget.h"
#include "console_widget/results_view.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "utils.h"
#include "managers/icon_manager.h"

#include <QAction>
#include <QHeaderView>
#include <QMenu>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

enum PolicyResultsColumn {
    PolicyResultsColumn_Name,
    PolicyResultsColumn_Enforced,
    PolicyResultsColumn_Disabled,
    PolicyResultsColumn_Path,

    PolicyResultsColumn_COUNT,
};

enum PolicyResultsRole {
    PolicyResultsRole_DN = Qt::UserRole,
    PolicyResultsRole_GplinkString = Qt::UserRole + 1,

    PolicyResultsRole_COUNT,
};

const QSet<PolicyResultsColumn> option_columns = {
    PolicyResultsColumn_Disabled,
    PolicyResultsColumn_Enforced,
};

const QHash<PolicyResultsColumn, GplinkOption> column_to_option = {
    {PolicyResultsColumn_Disabled, GplinkOption_Disabled},
    {PolicyResultsColumn_Enforced, GplinkOption_Enforced},
};

PolicyResultsWidget::PolicyResultsWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::PolicyResultsWidget();
    ui->setupUi(this);

    auto delete_link_action = new QAction(tr("Delete link"), this);

    context_menu = new QMenu(this);
    context_menu->addAction(delete_link_action);

    model = new QStandardItemModel(0, PolicyResultsColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model,
        {
            {PolicyResultsColumn_Name, tr("Location")},
            {PolicyResultsColumn_Enforced, tr("Enforced")},
            {PolicyResultsColumn_Disabled, tr("Disabled")},
            {PolicyResultsColumn_Path, tr("Path")},
        });

    ui->view->set_model(model);

    ui->view->detail_view()->header()->resizeSection(0, 300);
    ui->view->detail_view()->header()->resizeSection(1, 100);
    ui->view->detail_view()->header()->resizeSection(2, 100);
    ui->view->detail_view()->header()->resizeSection(3, 500);

    const QVariant state = settings_get_variant(SETTING_policy_results_state);
    ui->view->restore_state(state,
        {
            PolicyResultsColumn_Name,
            PolicyResultsColumn_Enforced,
            PolicyResultsColumn_Disabled,
            PolicyResultsColumn_Path,
        });

    connect(
        model, &QStandardItemModel::itemChanged,
        this, &PolicyResultsWidget::on_item_changed);
    connect(
        ui->view, &ResultsView::context_menu,
        this, &PolicyResultsWidget::open_context_menu);
    connect(
        delete_link_action, &QAction::triggered,
        this, &PolicyResultsWidget::delete_link);
}

PolicyResultsWidget::~PolicyResultsWidget() {
    const QVariant state = ui->view->save_state();
    settings_set_variant(SETTING_policy_results_state, state);

    delete ui;
}

void PolicyResultsWidget::update(const QModelIndex &index) {
    const ItemType type = (ItemType) console_item_get_type(index);
    if (type != ItemType_Policy) {
        return;
    }

    const QString new_gpo = index.data(PolicyRole_DN).toString();

    update(new_gpo);
}

void PolicyResultsWidget::update(const QString &new_gpo) {
    gpo = new_gpo;

    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    model->removeRows(0, model->rowCount());

    const QString base = g_adconfig->domain_dn();
    const SearchScope scope = SearchScope_All;
    const QList<QString> attributes = {ATTRIBUTE_NAME, ATTRIBUTE_GPLINK, ATTRIBUTE_OBJECT_CATEGORY};
    const QString filter = filter_CONDITION(Condition_Contains, ATTRIBUTE_GPLINK, gpo);
    const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

    for (const AdObject &object : results.values()) {
        const QList<QStandardItem *> row = make_item_row(PolicyResultsColumn_COUNT);

        const QString dn = object.get_dn();
        const QString name = dn_get_name(dn);
        row[PolicyResultsColumn_Name]->setText(name);

        row[PolicyResultsColumn_Path]->setText(dn_get_parent_canonical(dn));

        const QString gplink_string = object.get_string(ATTRIBUTE_GPLINK);
        const Gplink gplink = Gplink(gplink_string);

        const Qt::CheckState enforced_checkstate = [&]() {
            const bool is_enforced = gplink.get_option(gpo, GplinkOption_Enforced);
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

            const Qt::CheckState checkstate = [=]() {
                const GplinkOption option = column_to_option[column];
                const bool option_is_set = gplink.get_option(gpo, option);
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
        const QIcon icon = g_icon_manager->get_object_icon(object);
        row[0]->setIcon(icon);

        model->appendRow(row);
    }
}

ResultsView *PolicyResultsWidget::get_view() const {
    return ui->view;
}

QString PolicyResultsWidget::get_current_gpo() const {
    return gpo;
}

void PolicyResultsWidget::on_item_changed(QStandardItem *item) {
    const PolicyResultsColumn column = (PolicyResultsColumn) item->column();
    if (!option_columns.contains(column)) {
        return;
    }

    const QModelIndex this_index = item->index();
    const QModelIndex index = this_index.siblingAtColumn(0);
    const QString ou_dn = index.data(PolicyResultsRole_DN).toString();
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
    if (ad_failed(ad, this)) {
        return;
    }

    show_busy_indicator();

    const bool success = ad.attribute_replace_string(ou_dn, ATTRIBUTE_GPLINK, updated_gplink_string);

    if (success) {
        model->setData(index, updated_gplink_string, PolicyResultsRole_GplinkString);
        emit ou_gplink_changed(ou_dn, gplink, gpo, option);

    } else {
        const Qt::CheckState undo_check_state = [&]() {
            if (item->checkState() == Qt::Checked) {
                return Qt::Unchecked;
            } else {
                return Qt::Checked;
            }
        }();
        item->setCheckState(undo_check_state);
    }

    g_status->display_ad_messages(ad, this);

    hide_busy_indicator();
}

void PolicyResultsWidget::open_context_menu(const QPoint &pos) {
    const QModelIndex index = ui->view->current_view()->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    const QPoint global_pos = ui->view->current_view()->mapToGlobal(pos);
    context_menu->popup(global_pos);
}

void PolicyResultsWidget::delete_link() {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    show_busy_indicator();

    const QList<QModelIndex> selected = ui->view->get_selected_indexes();

    QList<QPersistentModelIndex> removed_indexes;

    for (const QModelIndex &index : selected) {
        const QString dn = index.data(PolicyResultsRole_DN).toString();

        const QString gplink_string = index.data(PolicyResultsRole_GplinkString).toString();
        Gplink gplink = Gplink(gplink_string);
        gplink.remove(gpo);
        const QString updated_gplink_string = gplink.to_string();

        const bool gplink_didnt_change = gplink.equals(gplink_string);
        if (gplink_didnt_change) {
            continue;
        }

        const bool success = ad.attribute_replace_string(dn, ATTRIBUTE_GPLINK, updated_gplink_string);

        if (success) {
            removed_indexes.append(QPersistentModelIndex(index));
            emit ou_gplink_changed(dn, gplink, gpo);
        }
    }

    for (const QPersistentModelIndex &index : removed_indexes) {
        model->removeRow(index.row());
    }

    g_status->display_ad_messages(ad, this);

    hide_busy_indicator();
}
