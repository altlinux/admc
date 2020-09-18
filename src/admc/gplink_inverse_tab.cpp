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

#include "gplink_inverse_tab.h"
#include "details_widget.h"
#include "utils.h"
#include "dn_column_proxy.h"
#include "ad_interface.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QMenu>

// TODO: giving up on providing a way to edit from here, too complicated. Instead user needs to go to one of the object's details and edit from there. Complicated because adding/removing modifies attribute of *object* NOT of the *gpo*. Also no way to see or modify order, etc.

enum GplinkInverseColumn {
    GplinkInverseColumn_Name,
    GplinkInverseColumn_DN,
    GplinkInverseColumn_COUNT
};

GplinkInverseTab::GplinkInverseTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setAllColumnsShowFocus(true);

    model = new QStandardItemModel(0, GplinkInverseColumn_COUNT, this);
    model->setHorizontalHeaderItem(GplinkInverseColumn_Name, new QStandardItem(tr("Name")));
    model->setHorizontalHeaderItem(GplinkInverseColumn_DN, new QStandardItem(tr("DN")));

    const auto dn_column_proxy = new DnColumnProxy(GplinkInverseColumn_DN, this);

    setup_model_chain(view, model, {dn_column_proxy});

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(view);

    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &GplinkInverseTab::on_context_menu);
}

bool GplinkInverseTab::changed() const {
    return false;
}

bool GplinkInverseTab::verify() {
    return true;
}

void GplinkInverseTab::apply() {

}

void GplinkInverseTab::reload() {
    model->removeRows(0, model->rowCount());

    const QString filter = filter_EQUALS(ATTRIBUTE_GPLINK, "*");
    const QList<QString> all_linked_objects = AdInterface::instance()->search(filter);

    for (auto dn : all_linked_objects) {
        const QString gplink = AdInterface::instance()->attribute_get(dn, ATTRIBUTE_GPLINK);
        const bool linked_to_this = gplink.contains(target());

        if (linked_to_this) {
            const QString name = AdInterface::instance()->get_name_for_display(dn);

            const QList<QStandardItem *> row = make_item_row(GplinkInverseColumn_COUNT);
            row[GplinkInverseColumn_Name]->setText(name);
            row[GplinkInverseColumn_DN]->setText(dn);

            model->appendRow(row);
        }
    }
}

bool GplinkInverseTab::accepts_target() const {
    const bool is_policy = AdInterface::instance()->is_policy(target());

    return is_policy;
}

// TODO: similar to code in ObjectContextMenu
void GplinkInverseTab::on_context_menu(const QPoint pos) {
    const QModelIndex base_index = view->indexAt(pos);
    if (!base_index.isValid()) {
        return;
    }
    const QModelIndex index = convert_to_source(base_index);
    const QString dn = get_dn_from_index(index, GplinkInverseColumn_DN);

    const QPoint global_pos = view->mapToGlobal(pos);

    auto menu = new QMenu(this);
    menu->addAction(tr("Details"), [dn]() {
        DetailsWidget::change_target(dn);
    });
    menu->popup(global_pos);
}
