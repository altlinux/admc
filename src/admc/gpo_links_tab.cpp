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

#include "gpo_links_tab.h"
#include "details_widget.h"
#include "utils.h"
#include "ad_interface.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QMenu>

// TODO: giving up on providing a way to edit from here, too complicated. Instead user needs to go to one of the object's details and edit from there. Complicated because adding/removing modifies attribute of *object* NOT of the *gpo*. Also no way to see or modify order, etc.

enum GpoLinksColumn {
    GpoLinksColumn_Name,
    GpoLinksColumn_DN,
    GpoLinksColumn_COUNT
};

GpoLinksTab::GpoLinksTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setAllColumnsShowFocus(true);

    model = new QStandardItemModel(0, GpoLinksColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model, {
        {GpoLinksColumn_Name, tr("Name")},
        {GpoLinksColumn_DN, tr("DN")}
    });

    view->setModel(model);

    setup_column_toggle_menu(view, model, {GpoLinksColumn_Name});

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(view);

    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &GpoLinksTab::on_context_menu);
}

bool GpoLinksTab::changed() const {
    return false;
}

bool GpoLinksTab::verify() {
    return true;
}

void GpoLinksTab::apply() {

}

void GpoLinksTab::reload(const Attributes &attributes) {
    model->removeRows(0, model->rowCount());

    // TODO: do this with search
    
    // NOTE: *target* means searching for gplink containing target
    const QList<QString> search_attributes = {ATTRIBUTE_NAME};
    const QString filter = filter_EQUALS(ATTRIBUTE_GPLINK, "*" + target() + "*");
    const QHash<QString, Attributes> search_results = AdInterface::instance()->search(filter, search_attributes, SearchScope_All);

    for (auto dn : search_results.keys()) {
        const QString name = attributes[ATTRIBUTE_NAME][0];

        const QList<QStandardItem *> row = make_item_row(GpoLinksColumn_COUNT);
        row[GpoLinksColumn_Name]->setText(name);
        row[GpoLinksColumn_DN]->setText(dn);

        model->appendRow(row);
    }
}

bool GpoLinksTab::accepts_target(const Attributes &attributes) const {
    const bool is_policy = object_is_policy(attributes);

    return is_policy;
}

void GpoLinksTab::on_context_menu(const QPoint pos) {
    const QString dn = get_dn_from_pos(pos, view, GpoLinksColumn_DN);

    QMenu menu(this);
    menu.addAction(tr("Details"), [dn]() {
        DetailsWidget::change_target(dn);
    });

    exec_menu_from_view(&menu, view, pos);
}
