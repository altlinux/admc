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

#include "policy_ou_results_widget.h"
#include "policy_ou_results_widget_p.h"
#include "ui_policy_ou_results_widget.h"

#include "adldap.h"
#include "console_impls/item_type.h"
#include "console_impls/policy_impl.h"
#include "console_impls/policy_ou_impl.h"
#include "console_impls/policy_root_impl.h"
#include "console_widget/console_widget.h"
#include "console_widget/results_view.h"
#include "globals.h"
#include "gplink.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QAction>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QTreeView>

enum PolicyOUResultsRole {
    PolicyOUResultsRole_DN = Qt::UserRole + 1,

    PolicyOUResultsRole_COUNT,
};

const QSet<PolicyOUResultsColumn> option_columns = {
    PolicyOUResultsColumn_Enforced,
    PolicyOUResultsColumn_Disabled,
};

const QHash<PolicyOUResultsColumn, GplinkOption> column_to_option = {
    {PolicyOUResultsColumn_Enforced, GplinkOption_Enforced},
    {PolicyOUResultsColumn_Disabled, GplinkOption_Disabled},
};

QString gplink_option_to_display_string(const QString &option);

PolicyOUResultsWidget::PolicyOUResultsWidget(ConsoleWidget *console_arg)
: QWidget(console_arg) {
    ui = new Ui::PolicyOUResultsWidget();
    ui->setupUi(this);

    console = console_arg;

    auto remove_link_action = new QAction(tr("Remove link"), this);
    auto move_up_action = new QAction(tr("Move up"), this);
    auto move_down_action = new QAction(tr("Move down"), this);

    context_menu = new QMenu(this);
    context_menu->addAction(remove_link_action);
    context_menu->addAction(move_up_action);
    context_menu->addAction(move_down_action);

    model = new QStandardItemModel(0, PolicyOUResultsColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model,
        {
            {PolicyOUResultsColumn_Order, tr("Order")},
            {PolicyOUResultsColumn_Name, tr("Name")},
            {PolicyOUResultsColumn_Enforced, tr("Enforced")},
            {PolicyOUResultsColumn_Disabled, tr("Disabled")},
        });

    ui->view->set_model(model);

    ui->view->detail_view()->header()->resizeSection(0, 50);
    ui->view->detail_view()->header()->resizeSection(1, 300);
    ui->view->detail_view()->header()->resizeSection(2, 100);
    ui->view->detail_view()->header()->resizeSection(3, 100);
    ui->view->detail_view()->header()->resizeSection(4, 500);

    const QVariant state = settings_get_variant(SETTING_policy_ou_results_state);
    ui->view->restore_state(state,
        {
            PolicyOUResultsColumn_Order,
            PolicyOUResultsColumn_Name,
            PolicyOUResultsColumn_Enforced,
            PolicyOUResultsColumn_Disabled,
        });

    connect(
        model, &QStandardItemModel::itemChanged,
        this, &PolicyOUResultsWidget::on_item_changed);
    connect(
        ui->view, &ResultsView::context_menu,
        this, &PolicyOUResultsWidget::open_context_menu);
    connect(
        remove_link_action, &QAction::triggered,
        this, &PolicyOUResultsWidget::remove_link);
    connect(
        move_up_action, &QAction::triggered,
        this, &PolicyOUResultsWidget::move_up);
    connect(
        move_down_action, &QAction::triggered,
        this, &PolicyOUResultsWidget::move_down);
}

PolicyOUResultsWidget::~PolicyOUResultsWidget() {
    const QVariant state = ui->view->save_state();
    settings_set_variant(SETTING_policy_ou_results_state, state);

    delete ui;
}

void PolicyOUResultsWidget::update(const QModelIndex &index) {
    const ItemType type = (ItemType) console_item_get_type(index);
    if (type != ItemType_PolicyOU) {
        return;
    }

    const QString dn = index.data(PolicyOURole_DN).toString();

    update(dn);
}

void PolicyOUResultsWidget::update(const QString &dn) {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    ou_dn = dn;

    gplink = [&]() {
        const AdObject object = ad.search_object(ou_dn);
        const QString gplink_string = object.get_string(ATTRIBUTE_GPLINK);
        const Gplink out = Gplink(gplink_string);

        return out;
    }();

    reload_gplink();
}

ResultsView *PolicyOUResultsWidget::get_view() const {
    return ui->view;
}

void PolicyOUResultsWidget::on_item_changed(QStandardItem *item) {
    const PolicyOUResultsColumn column = (PolicyOUResultsColumn) item->column();
    if (!option_columns.contains(column)) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    show_busy_indicator();

    const QModelIndex this_index = item->index();
    const QModelIndex index = this_index.siblingAtColumn(0);
    const QString gpo_dn = index.data(PolicyOUResultsRole_DN).toString();
    const GplinkOption option = column_to_option[column];
    const bool is_checked = (item->checkState() == Qt::Checked);

    gplink.set_option(gpo_dn, option, is_checked);

    const QString gplink_string = gplink.to_string();

    bool success = ad.attribute_replace_string(ou_dn, ATTRIBUTE_GPLINK, gplink_string);

    if (success && option == GplinkOption_Enforced)
        set_policy_enforced_icon(gpo_dn, is_checked);

    g_status->display_ad_messages(ad, this);

    reload_gplink();

    hide_busy_indicator();
}

void PolicyOUResultsWidget::open_context_menu(const QPoint &pos) {
    const QModelIndex index = ui->view->current_view()->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    const QPoint global_pos = ui->view->current_view()->mapToGlobal(pos);
    context_menu->popup(global_pos);
}

// Takes as argument function that modifies the gplink.
// Modify function accepts gplink as argument and gpo dn
// used in the operation.
void PolicyOUResultsWidget::modify_gplink(void (*modify_function)(Gplink &, const QString &)) {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    show_busy_indicator();

    const QList<QModelIndex> selected = ui->view->get_selected_indexes();

    const QString old_gplink_string = gplink.to_string();

    for (const QModelIndex &index : selected) {
        const QString gpo_dn = index.data(PolicyOUResultsRole_DN).toString();

        modify_function(gplink, gpo_dn);
    }

    const QString gplink_string = gplink.to_string();
    ad.attribute_replace_string(ou_dn, ATTRIBUTE_GPLINK, gplink_string);

    g_status->display_ad_messages(ad, this);

    reload_gplink();

    hide_busy_indicator();
}

void PolicyOUResultsWidget::set_policy_enforced_icon(const QString &policy_dn, bool is_enforced)
{
    QModelIndex target_policy_index = console->search_item(console->get_current_scope_item(),
                                                             PolicyRole_DN,
                                                             policy_dn,
                                                             {ItemType_Policy});
    if (!target_policy_index.isValid())
        return;

    if (is_enforced)
        console->get_item(target_policy_index)->setIcon(
                    target_policy_index.data(PolicyRole_Enforced_Icon).value<QIcon>());
    else
        console->get_item(target_policy_index)->setIcon(
                    target_policy_index.data(PolicyRole_Clean_Icon).value<QIcon>());
}

void PolicyOUResultsWidget::remove_link() {
    // NOTE: save gpo dn list before they are removed in
    // modify_gplink()
    const QList<QString> gpo_dn_list = [&]() {
        QList<QString> out;

        const QList<QModelIndex> selected = ui->view->get_selected_indexes();

        for (const QModelIndex &index : selected) {
            const QString gpo_dn = index.data(PolicyOUResultsRole_DN).toString();

            out.append(gpo_dn);
        }

        return out;
    }();

    auto modify_function = [](Gplink &gplink_arg, const QString &gpo) {
        gplink_arg.remove(gpo);
    };

    modify_gplink(modify_function);

    // Also remove gpo from OU in console
    const QModelIndex policy_root = get_policy_tree_root(console);
    if (policy_root.isValid()) {
        const QModelIndex ou_index = console->search_item(policy_root, PolicyOURole_DN, ou_dn, {ItemType_PolicyOU});

        if (ou_index.isValid()) {
            for (const QString &gpo_dn : gpo_dn_list) {
                const QModelIndex gpo_index = console->search_item(ou_index, PolicyRole_DN, gpo_dn, {ItemType_Policy});

                if (gpo_index.isValid()) {
                    console->delete_item(gpo_index);
                }
            }
        }
    }
}

void PolicyOUResultsWidget::move_up() {
    auto modify_function = [](Gplink &gplink_arg, const QString &gpo) {
        gplink_arg.move_up(gpo);
    };

    modify_gplink(modify_function);
}

void PolicyOUResultsWidget::move_down() {
    auto modify_function = [](Gplink &gplink_arg, const QString &gpo) {
        gplink_arg.move_down(gpo);
    };

    modify_gplink(modify_function);
}

void PolicyOUResultsWidget::reload_gplink() {
    model->removeRows(0, model->rowCount());

    AdInterface ad;
    if (ad_failed(ad, ui->view)) {
        return;
    }

    const QList<QString> gpo_dn_list = gplink.get_gpo_list();

    const QList<AdObject> gpo_object_list = [&]() {
        if (gpo_dn_list.isEmpty()) {
            return QList<AdObject>();
        }

        const QString base = g_adconfig->policies_dn();
        const SearchScope scope = SearchScope_Children;
        const QString filter = filter_dn_list(gpo_dn_list);
        const QList<QString> attributes = QList<QString>();

        const QHash<QString, AdObject> search_results = ad.search(base, scope, filter, attributes);

        const QList<AdObject> out = search_results.values();

        return out;
    }();

    for (const AdObject &gpo_object : gpo_object_list) {
        // NOTE: Gplink may contain invalid gpo's, in which
        // case there's no real object for the policy, but
        // we still show the broken object to the user so he
        // is aware of it.
        const bool gpo_is_valid = !gpo_object.is_empty();
        const QString gpo_dn = gpo_object.get_dn();

        const QString name = [&]() {
            if (gpo_is_valid) {
                const QString display_name = gpo_object.get_string(ATTRIBUTE_DISPLAY_NAME);

                return display_name;
            } else {
                return tr("Not found");
            }
        }();

        const QList<QStandardItem *> row = make_item_row(PolicyOUResultsColumn_COUNT);

        const int index = gpo_dn_list.indexOf(gpo_object.get_dn());
        const QString index_string = QString::number(index);
        row[PolicyOUResultsColumn_Order]->setText(index_string);

        row[PolicyOUResultsColumn_Name]->setText(name);
        set_data_for_row(row, gpo_dn, PolicyOUResultsRole_DN);

        const QIcon icon = get_object_icon(gpo_object);
        row[0]->setIcon(icon);

        for (const auto column : option_columns) {
            QStandardItem *item = row[column];
            item->setCheckable(true);

            const Qt::CheckState checkstate = [=]() {
                const GplinkOption option = column_to_option[column];
                const bool option_is_set = gplink.get_option(gpo_dn, option);
                if (option_is_set) {
                    return Qt::Checked;
                } else {
                    return Qt::Unchecked;
                }
            }();
            item->setCheckState(checkstate);
        }

        if (!gpo_is_valid) {
            row[PolicyOUResultsColumn_Name]->setIcon(QIcon::fromTheme("dialog-error"));

            for (QStandardItem *item : row) {
                item->setToolTip(tr("The GPO for this link could not be found. It maybe have been recently created and is being replicated or it could have been deleted."));
            }
        }

        model->appendRow(row);
    }

    model->sort(PolicyOUResultsColumn_Order);
}
