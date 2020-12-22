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

#include "contents_widget.h"
#include "settings.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "object_model.h"
#include "advanced_view_proxy.h"
#include "object_context_menu.h"
#include "details_dialog.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QTreeView>
#include <QHeaderView>
#include <QLabel>

ContentsWidget::ContentsWidget(ObjectModel *model_arg)
: QWidget()
{   
    model = model_arg;

    view = new QTreeView(this);
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setRootIsDecorated(false);
    view->setItemsExpandable(false);
    view->setExpandsOnDoubleClick(false);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);
    view->header()->setSectionsMovable(true);

    DetailsDialog::connect_to_open_by_double_click(view, ADCONFIG()->get_column_index(ATTRIBUTE_DN));

    advanced_view_proxy = new AdvancedViewProxy(this);

    advanced_view_proxy->setSourceModel(model);
    view->setModel(advanced_view_proxy);

    setup_column_toggle_menu(view, model, {ADCONFIG()->get_column_index(ATTRIBUTE_NAME), ADCONFIG()->get_column_index(ATTRIBUTE_OBJECT_CLASS), ADCONFIG()->get_column_index(ATTRIBUTE_DESCRIPTION)});

    header = new QWidget();

    header_label = new QLabel();

    auto header_layout = new QVBoxLayout();
    header->setLayout(header_layout);
    header_layout->setContentsMargins(0, 0, 0, 0);
    header_layout->setSpacing(0);

    header_layout->addWidget(header_label);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(header);
    layout->addWidget(view);

    ObjectContextMenu::setup_as_context_menu(view, ADCONFIG()->get_column_index(ATTRIBUTE_DN));

    const BoolSettingSignal *show_header_signal = SETTINGS()->get_bool_signal(BoolSetting_ShowContentsHeader);
    connect(
        show_header_signal, &BoolSettingSignal::changed,
        this, &ContentsWidget::on_header_toggled);
    on_header_toggled();

    connect(
        view->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &ContentsWidget::emit_current_changed);
}

void ContentsWidget::on_containers_selected_changed(const QModelIndex &source_index) {
    // NOTE: need to fetchMore() before setRootIndex() otherwise header gets messed up
    model->fetchMore(source_index);

    const bool no_children = !model->hasChildren(source_index);

    // NOTE: MASSIVE BANDAID. setRootIndex() on an object without children causes header to disappear and lose state(column visibility and widths). To fix this, before changing root index, add an empty row, then change root index, header loads normally, remove empty row, header stays there.
    // NOTE: this seems to be guaranteed to cause problems in the future. Maybe something to do with signals/slots related to model changes, so watch this.
    if (no_children) {
        auto item = model->itemFromIndex(source_index);
        const QList<QStandardItem *> row = make_item_row(ADCONFIG()->get_columns().size());
        item->appendRow(row);
    }

    const QModelIndex advanced_view_proxy_index = advanced_view_proxy->mapFromSource(source_index);
    view->setRootIndex(advanced_view_proxy_index);

    // Remove that empty row we created to fix header
    if (no_children) {
        auto item = model->itemFromIndex(source_index);
        item->removeRow(0);
    }

    const QString label_text =
    [this, source_index]() {
        const QString object_count_string = tr("%n object(s)", "", model->rowCount(source_index));

        const QString parent_name = source_index.data().toString();

        return QString("%1: %2").arg(parent_name, object_count_string);
    }();
    header_label->setText(label_text);
}

void ContentsWidget::on_header_toggled() {
    const bool show_header = SETTINGS()->get_bool(BoolSetting_ShowContentsHeader);
    header->setVisible(show_header);
}

void ContentsWidget::emit_current_changed(const QModelIndex &current, const QModelIndex &) {
    const QString &dn = get_dn_from_index(current, ADCONFIG()->get_column_index(ATTRIBUTE_DN));

    emit current_changed(dn);
}

void ContentsWidget::showEvent(QShowEvent *event) {
    resize_columns(view,
    {
        {ADCONFIG()->get_column_index(ATTRIBUTE_NAME), 0.4},
        {ADCONFIG()->get_column_index(ATTRIBUTE_OBJECT_CLASS), 0.4},
    });
}
