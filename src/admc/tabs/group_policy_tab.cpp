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
#include "ad/ad_interface.h"
#include "ad/ad_utils.h"
#include "ad/ad_object.h"
#include "utils.h"
#include "select_dialog.h"
#include "edits/gpoptions_edit.h"
#include "ad/ad_filter.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QStandardItemModel>
#include <QMenu>
#include <QPushButton>
#include <QDebug>

enum GplinkColumn {
    GplinkColumn_Name,
    GplinkColumn_Disabled,
    GplinkColumn_Enforced,
    GplinkColumn_COUNT
};

enum GplinkRole {
    GplinkRole_DN = Qt::UserRole + 1,
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
    });

    view->setModel(model);

    const auto edits_layout = new QFormLayout();

    new GpoptionsEdit(&edits, this);
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

    enable_widget_on_selection(remove_button, view);

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

void GroupPolicyTab::load(AdInterface &ad, const AdObject &object) {
    const QString gplink_string = object.get_string(ATTRIBUTE_GPLINK);
    gplink = Gplink(gplink_string);
    
    reload_gplink();
    
    PropertiesTab::load(ad, object);
}

bool GroupPolicyTab::apply(AdInterface &ad, const QString &target) {
    bool total_success = true;

    const QString gplink_string = gplink.to_string();
    const bool replace_success = ad.attribute_replace_string(target, ATTRIBUTE_GPLINK, gplink_string);
    if (!replace_success) {
        total_success = false;
    }

    const bool apply_success = PropertiesTab::apply(ad, target);
    if (!apply_success) {
        total_success = false;
    }
    
    return total_success;
}

void GroupPolicyTab::on_context_menu(const QPoint pos) {
    const QModelIndex index = view->indexAt(pos);
    const QString gpo = index.data(GplinkRole_DN).toString();
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
    auto dialog = new SelectDialog({CLASS_GP_CONTAINER}, SelectDialogMultiSelection_Yes, this);

    const QString title = tr("Add policy link");
    dialog->setWindowTitle(title);

    connect(
        dialog, &SelectDialog::accepted,
        [this, dialog]() {
            const QList<QString> selected = dialog->get_selected();
            
            add_link(selected);
        });

    dialog->open();
}

void GroupPolicyTab::on_remove_button() {
    const QItemSelectionModel *selection_model = view->selectionModel();
    const QList<QModelIndex> selected_raw = selection_model->selectedRows();

    QList<QString> selected;
    for (auto index : selected_raw) {
        const QString gpo = index.data(GplinkRole_DN).toString();

        selected.append(gpo);
    }

    remove_link(selected);    
}

void GroupPolicyTab::add_link(QList<QString> gps) {
    for (auto gp : gps) {
        gplink.add(gp);
    }

    reload_gplink();

    emit edited();
}

void GroupPolicyTab::remove_link(QList<QString> gps) {
    for (auto gp : gps) {
        gplink.remove(gp);
    }

    reload_gplink();

    emit edited();
}

void GroupPolicyTab::move_link_up(const QString &gpo) {
    gplink.move_up(gpo);

    reload_gplink();

    emit edited();
}

void GroupPolicyTab::move_link_down(const QString &gpo) {
    gplink.move_down(gpo);

    reload_gplink();

    emit edited();
}

void GroupPolicyTab::reload_gplink() {
    model->removeRows(0, model->rowCount());

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    // TODO: use filter to search only for needed gpo's, not all of them (dn=dn1 or dn=dn2 or ...)
    const QList<QString> search_attributes = {ATTRIBUTE_DISPLAY_NAME};
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);
    const QHash<QString, AdObject> search_results = ad.search(filter, search_attributes, SearchScope_All);

    const QList<QString> gpos = gplink.get_gpos();

    for (auto dn : search_results.keys()) {
        if (!gpos.contains(dn)) {
            continue;
        }

        const AdObject object  = search_results[dn];

        const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);

        const QList<QStandardItem *> row = make_item_row(GplinkColumn_COUNT);
        row[GplinkColumn_Name]->setText(display_name);
        set_data_for_row(row, dn, GplinkRole_DN);

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

        model->appendRow(row);
    }

    model->sort(GplinkColumn_Name);
}

void GroupPolicyTab::on_item_changed(QStandardItem *item) {
    const GplinkColumn column = (GplinkColumn) item->column();

    if (option_columns.contains(column)) {
        const QModelIndex index = item->index();
        const QString gpo = index.data(GplinkRole_DN).toString();
        const GplinkOption option = column_to_option[column];
        const bool is_checked = (item->checkState() == Qt::Checked);

        gplink.set_option(gpo, option, is_checked);

        reload_gplink();
    }
}
