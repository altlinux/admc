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

#include "move_dialog.h"

#include "object_model.h"
#include "containers_proxy.h"
#include "advanced_view_proxy.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "ad_defines.h"
#include "utils.h"
#include "status.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

MoveDialog::MoveDialog(const QList<QString> targets_arg, QWidget *parent)
: QDialog(parent)
{
    targets = targets_arg;
    
    setAttribute(Qt::WA_DeleteOnClose);
    
    resize(400, 500);

    auto model = new ObjectModel(this);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setExpandsOnDoubleClick(true);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);
    view->sortByColumn(ADCONFIG()->get_column_index(ATTRIBUTE_NAME), Qt::AscendingOrder);

    auto advanced_view_proxy = new AdvancedViewProxy(this);
    auto containers_proxy = new ContainersProxy(this);

    containers_proxy->setSourceModel(model);
    advanced_view_proxy->setSourceModel(containers_proxy);
    view->setModel(advanced_view_proxy);

    setup_column_toggle_menu(view, model, {ADCONFIG()->get_column_index(ATTRIBUTE_NAME)});

    auto buttonbox = new QDialogButtonBox();
    auto ok_button = buttonbox->addButton(QDialogButtonBox::Ok);
    buttonbox->addButton(QDialogButtonBox::Cancel);

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

void MoveDialog::accept() {
    const QModelIndex selected = view->selectionModel()->currentIndex();
    const QString container = get_dn_from_index(selected, ADCONFIG()->get_column_index(ATTRIBUTE_DN));

    STATUS()->start_error_log();

    for (const QString target : targets) {
        AD()->object_move(target, container);
    }

    STATUS()->end_error_log(parentWidget());

    QDialog::accept();
}

void MoveDialog::showEvent(QShowEvent *event) {
    resize_columns(view,
    {
        {ADCONFIG()->get_column_index(ATTRIBUTE_NAME), 0.5},
        {ADCONFIG()->get_column_index(ATTRIBUTE_DN), 0.5},
    });
}
