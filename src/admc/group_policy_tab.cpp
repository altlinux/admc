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
#include "dn_column_proxy.h"
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
    GplinkColumn_Option,
    GplinkColumn_DN,
    GplinkColumn_COUNT
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
    model->setHorizontalHeaderItem(GplinkColumn_Name, new QStandardItem(tr("Name")));
    model->setHorizontalHeaderItem(GplinkColumn_Option, new QStandardItem(tr("Option")));
    model->setHorizontalHeaderItem(GplinkColumn_DN, new QStandardItem(tr("DN")));

    const auto dn_column_proxy = new DnColumnProxy(GplinkColumn_DN, this);

    setup_model_chain(view, model, {dn_column_proxy});

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

void GroupPolicyTab::reload() {
    const QString gplink_string = AdInterface::instance()->attribute_get(target(), ATTRIBUTE_GPLINK);
    original_gplink = Gplink(gplink_string);
    current_gplink = original_gplink;

    load_attribute_edits(edits, target());

    edited();
}

// TODO: not sure which object classes can have gplink, for now only know of OU's.
bool GroupPolicyTab::accepts_target() const {
    const bool is_ou = AdInterface::instance()->is_ou(target());

    return is_ou;
}

// TODO: similar to code in ObjectContextMenu
void GroupPolicyTab::on_context_menu(const QPoint pos) {
    const QModelIndex base_index = view->indexAt(pos);
    if (!base_index.isValid()) {
        return;
    }
    const QModelIndex index = convert_to_source(base_index);
    const QString gpo = get_dn_from_index(index, GplinkColumn_DN);

    const QPoint global_pos = view->mapToGlobal(pos);

    auto menu = new QMenu(this);
    menu->addAction(tr("Remove link"), [this, gpo]() {
        const QList<QString> removed = {gpo};
        remove(removed);
        edited();
    });
    menu->addAction(tr("Move up"), [this, gpo]() {
        current_gplink.move_up(gpo);
        edited();
    });
    menu->addAction(tr("Move down"), [this, gpo]() {
        current_gplink.move_down(gpo);
        edited();
    });

    static const QSet<QString> options = {
        GPLINK_OPTION_NONE,
        GPLINK_OPTION_DISABLE,
        GPLINK_OPTION_ENFORCE
    };
    auto option_submenu = menu->addMenu(tr("Set option"));
    for (const auto option : options) {
        const QString option_display_string = gplink_option_to_display_string(option);
        option_submenu->addAction(option_display_string, [this, gpo, option]() {
            current_gplink.set_option(gpo, option);
            edited();
        });
    }
    
    menu->popup(global_pos);
}

void GroupPolicyTab::on_add_button() {
    const QList<QString> classes = {CLASS_GP_CONTAINER};
    const QList<QString> selected = SelectDialog::open(classes, SelectDialogMultiSelection_Yes);

    if (selected.size() > 0) {
        add(selected);
    }
}

void GroupPolicyTab::on_remove_button() {
    const QItemSelectionModel *selection_model = view->selectionModel();
    const QList<QModelIndex> selected_raw = selection_model->selectedIndexes();

    QList<QString> selected;
    for (auto index : selected_raw) {
        const QModelIndex converted = convert_to_source(index);
        const QString gpo = get_dn_from_index(converted, GplinkColumn_DN);

        selected.append(gpo);
    }

    remove(selected);    
}

void GroupPolicyTab::add(QList<QString> gps) {
    for (auto gp : gps) {
        current_gplink.add(gp);
    }

    edited();
}

void GroupPolicyTab::remove(QList<QString> gps) {
    for (auto gp : gps) {
        current_gplink.remove(gp);
    }

    edited();
}

// TODO: members tab needs this as well. DetailsTab::on_edit_changed() slot is weird in general. Also, idk if "edited" is a good name, sounds like a getter for a bool.
void GroupPolicyTab::edited() {
    model->removeRows(0, model->rowCount());

    for (auto gpo : current_gplink.get_gpos()) {
        const QString option = current_gplink.get_option(gpo);
        const QString option_display_string = gplink_option_to_display_string(option);
        const QString name = AdInterface::instance()->get_name_for_display(gpo);

        const QList<QStandardItem *> row = make_item_row(GplinkColumn_COUNT);
        row[GplinkColumn_Name]->setText(name);
        row[GplinkColumn_Option]->setText(option_display_string);
        row[GplinkColumn_DN]->setText(gpo);

        model->appendRow(row);
    }

    on_edit_changed();
}

QString gplink_option_to_display_string(const QString &option) {
    static const QHash<QString, QString> display_strings = {
        {GPLINK_OPTION_NONE, QObject::tr("None")},
        {GPLINK_OPTION_DISABLE, QObject::tr("Disable")},
        {GPLINK_OPTION_ENFORCE, QObject::tr("Enforce")}
    };
    if (display_strings.contains(option)) {
        const QString display_string = display_strings[option];
        return display_string;
    } else {
        return QObject::tr("UNKNOWN OPTION");
    }
}
