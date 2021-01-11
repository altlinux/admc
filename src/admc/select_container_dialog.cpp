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

#include "select_container_dialog.h"

#include "object_model.h"
#include "containers_proxy.h"
#include "advanced_view_proxy.h"
#include "utils.h"
#include "ad_config.h"
#include "ad_defines.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHeaderView>

SelectContainerDialog::SelectContainerDialog(QWidget *parent)
: QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    
    resize(400, 500);

    auto model = new ObjectModel(this);
    model->load_head_object();

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setExpandsOnDoubleClick(true);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);
    view->sortByColumn(ADCONFIG()->get_column_index(ATTRIBUTE_NAME), Qt::AscendingOrder);
    view->setHeaderHidden(true);

    auto advanced_view_proxy = new AdvancedViewProxy(this);
    auto containers_proxy = new ContainersProxy(this);

    containers_proxy->setSourceModel(model);
    advanced_view_proxy->setSourceModel(containers_proxy);
    view->setModel(advanced_view_proxy);

    auto buttonbox = new QDialogButtonBox();
    auto ok_button = buttonbox->addButton(QDialogButtonBox::Ok);
    buttonbox->addButton(QDialogButtonBox::Cancel);

    // Hide all columns except name column
    // TODO: this seems like it will end up getting duplicated somewhere so make a util f-n like
    QHeaderView *header = view->header();
    for (int i = 0; i < header->count(); i++) {
        header->setSectionHidden(i, true);
    }
    header->setSectionHidden(ADCONFIG()->get_column_index(ATTRIBUTE_NAME), false);

    connect(
        buttonbox, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        buttonbox, &QDialogButtonBox::rejected,
        this, &QDialog::reject);

    enable_widget_on_selection(ok_button, view);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(view);
    layout->addWidget(buttonbox);
}

QString SelectContainerDialog::get_selected() const {
    const QModelIndex selected_index = view->selectionModel()->currentIndex();
    const QString selected = get_dn_from_index(selected_index, ADCONFIG()->get_column_index(ATTRIBUTE_DN));

    return selected;
}
