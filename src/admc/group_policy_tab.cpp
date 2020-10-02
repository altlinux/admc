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

#include "group_policy_tab.h"
#include "object_context_menu.h"
#include "utils.h"
#include "select_dialog.h"
#include "edits/gpoptions_edit.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QMenu>
#include <QPushButton>
#include <QDebug>

enum GplinkColumn {
    GplinkColumn_Name,
    GplinkColumn_Disabled,
    GplinkColumn_Enforced,
    GplinkColumn_DN,
    GplinkColumn_COUNT
};

const QSet<GplinkColumn> option_columns = {
    GplinkColumn_Disabled,
    GplinkColumn_Enforced
};

const QHash<GplinkColumn, GplinkOption> column_to_option = {
    {GplinkColumn_Disabled, GplinkOption_Disabled},
    {GplinkColumn_Enforced, GplinkOption_Enforced}
};

QString gplink_option_to_display_string(const QString &option);

GroupPolicyTab::GroupPolicyTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setAllColumnsShowFocus(true);

    model = new QStandardItemModel(0, GplinkColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model, {
        {GplinkColumn_Name, tr("Name")},
        {GplinkColumn_Disabled, tr("Disabled")},
        {GplinkColumn_Enforced, tr("Enforced")},
        {GplinkColumn_DN, tr("DN")}
    });

    view->setModel(model);

    setup_column_toggle_menu(view, model, {GplinkColumn_Name, GplinkColumn_Disabled, GplinkColumn_Enforced});

    const auto edits_layout = new QGridLayout();

    auto gpoptions_edit = new GpoptionsEdit(this);
    edits.append(gpoptions_edit);
    layout_attribute_edits(edits, edits_layout);
    connect_edits_to_tab(edits, this);

    auto add_button = new QPushButton(tr("Add"));
    auto remove_button = new QPushButton(tr("Remove"));
    auto button_layout = new QHBoxLayout();
    button_layout->addWidget(add_button);
    button_layout->addWidget(remove_button);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);
    layout->addLayout(button_layout);
    layout->addLayout(edits_layout);

    connect(
        remove_button, &QAbstractButton::clicked,
        this, &GroupPolicyTab::on_remove_button);
    connect(
        add_button, &QAbstractButton::clicked,
        this, &GroupPolicyTab::on_add_button);
    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &GroupPolicyTab::on_context_menu);
    connect(
        model, &QStandardItemModel::itemChanged,
        this, &GroupPolicyTab::on_item_changed);
}

bool GroupPolicyTab::changed() const {
    const QString original_gplink_string = original_gplink.to_string();
    const QString current_gplink_string = current_gplink.to_string();
    const bool gplink_changed = (current_gplink_string != original_gplink_string);

    return any_edits_changed(edits) || gplink_changed;
}

bool GroupPolicyTab::verify() {
    return verify_attribute_edits(edits, this);
}

void GroupPolicyTab::apply() {
    const QString gplink_string = current_gplink.to_string();
    AdInterface::instance()->attribute_replace(target(), ATTRIBUTE_GPLINK, gplink_string);

    apply_attribute_edits(edits, target(), this);
}

void GroupPolicyTab::reload(const AttributesBinary &attributes) {
    const QString gplink_string(attributes[ATTRIBUTE_GPLINK][0]);
    original_gplink = Gplink(gplink_string);
    current_gplink = original_gplink;

    load_attribute_edits(edits, attributes);

    reload_current_gplink_into_model();
}

// TODO: not sure which object classes can have gplink, for now only know of OU's.
bool GroupPolicyTab::accepts_target(const AttributesBinary &attributes) const {
    const bool is_ou = is_ou2(attributes);

    return is_ou;
}

void GroupPolicyTab::on_context_menu(const QPoint pos) {
    const QString gpo = get_dn_from_pos(pos, view, GplinkColumn_DN);

    QMenu menu(this);
    menu.addAction(tr("Remove link"), [this, gpo]() {
        const QList<QString> removed = {gpo};
        remove_link(removed);
    });
    menu.addAction(tr("Move up"), [this, gpo]() {
        move_link_up(gpo);
    });
    menu.addAction(tr("Move down"), [this, gpo]() {
        move_link_down(gpo);
    });

    exec_menu_from_view(&menu, view, pos);
}

void GroupPolicyTab::on_add_button() {
    const QList<QString> classes = {CLASS_GP_CONTAINER};
    const QList<QString> selected = SelectDialog::open(classes, SelectDialogMultiSelection_Yes);

    if (selected.size() > 0) {
        add_link(selected);
    }
}

void GroupPolicyTab::on_remove_button() {
    const QItemSelectionModel *selection_model = view->selectionModel();
    const QList<QModelIndex> selected_raw = selection_model->selectedIndexes();

    QList<QString> selected;
    for (auto index : selected_raw) {
        const QString gpo = get_dn_from_index(index, GplinkColumn_DN);

        selected.append(gpo);
    }

    remove_link(selected);    
}

void GroupPolicyTab::add_link(QList<QString> gps) {
    for (auto gp : gps) {
        current_gplink.add(gp);
    }

    reload_current_gplink_into_model();
}

void GroupPolicyTab::remove_link(QList<QString> gps) {
    for (auto gp : gps) {
        current_gplink.remove(gp);
    }

    reload_current_gplink_into_model();
}

void GroupPolicyTab::move_link_up(const QString &gpo) {
    current_gplink.move_up(gpo);

    reload_current_gplink_into_model();
}

void GroupPolicyTab::move_link_down(const QString &gpo) {
    current_gplink.move_down(gpo);

    reload_current_gplink_into_model();
}

void GroupPolicyTab::reload_current_gplink_into_model() {
    model->removeRows(0, model->rowCount());

    // TODO: think i have to search filtering for all dn's?
    // "dn=d1 OR dn=d2 OR ..."

    // for (auto gpo : current_gplink.get_gpos()) {
    //     const QString name = AdInterface::instance()->get_name_for_display(gpo);

    //     const QList<QStandardItem *> row = make_item_row(GplinkColumn_COUNT);
    //     row[GplinkColumn_Name]->setText(name);
    //     row[GplinkColumn_DN]->setText(gpo);

    //     for (const auto column : option_columns) {
    //         QStandardItem *item = row[column];
    //         item->setCheckable(true);

    //         const GplinkOption option = column_to_option[column];
    //         const bool option_is_set = current_gplink.get_option(gpo, option);
    //         check_item_set_checked(item, option_is_set);
    //     }

    //     model->appendRow(row);
    // }
}

void GroupPolicyTab::on_item_changed(QStandardItem *item) {
    const GplinkColumn column = (GplinkColumn) item->column();

    if (option_columns.contains(column)) {
        const QModelIndex index = item->index();
        const QString gpo = get_dn_from_index(index, GplinkColumn_DN);
        const GplinkOption option = column_to_option[column];
        const bool is_checked = check_item_is_checked(item);

        current_gplink.set_option(gpo, option, is_checked);

        reload_current_gplink_into_model();
    }
}
