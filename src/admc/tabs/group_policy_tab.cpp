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

#include "tabs/group_policy_tab.h"
#include "object_context_menu.h"
#include "utils.h"
#include "select_dialog.h"
#include "edits/gpoptions_edit.h"
#include "filter.h"

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

GroupPolicyTab::GroupPolicyTab() {   
    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);

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

    new GpoptionsEdit(this, &edits);
    edits_add_to_layout(edits, edits_layout);
    edits_connect_to_tab(edits, this);

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

void GroupPolicyTab::load(const AdObject &object) {
    const QString gplink_string = object.get_string(ATTRIBUTE_GPLINK);
    original_gplink = Gplink(gplink_string);
    current_gplink = original_gplink;
    
    reload_model(original_gplink);
    
    DetailsTab::load(object);
}

bool GroupPolicyTab::changed() const {
    const QString original_gplink_string = original_gplink.to_string();
    const QString current_gplink_string = current_gplink.to_string();
    const bool gplink_changed = (current_gplink_string != original_gplink_string);

    return DetailsTab::changed() || gplink_changed;
}

void GroupPolicyTab::apply(const QString &target) const {
    const QString gplink_string = current_gplink.to_string();
    AD()->attribute_replace_string(target, ATTRIBUTE_GPLINK, gplink_string);

    DetailsTab::apply(target);
}

void GroupPolicyTab::on_context_menu(const QPoint pos) {
    const QString gpo = get_dn_from_pos(pos, view, GplinkColumn_DN);
    if (gpo.isEmpty()) {
        return;
    }

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

    reload_model(current_gplink);
}

void GroupPolicyTab::remove_link(QList<QString> gps) {
    for (auto gp : gps) {
        current_gplink.remove(gp);
    }

    reload_model(current_gplink);
}

void GroupPolicyTab::move_link_up(const QString &gpo) {
    current_gplink.move_up(gpo);

    reload_model(current_gplink);
}

void GroupPolicyTab::move_link_down(const QString &gpo) {
    current_gplink.move_down(gpo);

    reload_model(current_gplink);
}

void GroupPolicyTab::reload_model(const Gplink &gplink) {
    model->removeRows(0, model->rowCount());

    // TODO: use filter to search only for needed gpo's, not all of them (dn=dn1 or dn=dn2 or ...)
    const QList<QString> search_attributes = {ATTRIBUTE_DISPLAY_NAME};
    const QString filter = filter_EQUALS(ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);
    const QHash<QString, AdObject> search_results = AD()->search(filter, search_attributes, SearchScope_All);

    const QList<QString> gpos = gplink.get_gpos();

    for (auto dn : search_results.keys()) {
        if (!gpos.contains(dn)) {
            continue;
        }

        const AdObject object  = search_results[dn];

        const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);

        const QList<QStandardItem *> row = make_item_row(GplinkColumn_COUNT);
        row[GplinkColumn_Name]->setText(display_name);
        row[GplinkColumn_DN]->setText(dn);

        for (const auto column : option_columns) {
            QStandardItem *item = row[column];
            item->setCheckable(true);

            const GplinkOption option = column_to_option[column];
            const bool option_is_set = gplink.get_option(dn, option);
            check_item_set_checked(item, option_is_set);
        }

        model->appendRow(row);
    }

    model->sort(GplinkColumn_Name);
}

void GroupPolicyTab::on_item_changed(QStandardItem *item) {
    const GplinkColumn column = (GplinkColumn) item->column();

    if (option_columns.contains(column)) {
        const QModelIndex index = item->index();
        const QString gpo = get_dn_from_index(index, GplinkColumn_DN);
        const GplinkOption option = column_to_option[column];
        const bool is_checked = check_item_is_checked(item);

        current_gplink.set_option(gpo, option, is_checked);

        reload_model(current_gplink);
    }
}
