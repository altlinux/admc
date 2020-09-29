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

#include "containers_widget.h"
#include "advanced_view_proxy.h"
#include "object_context_menu.h"
#include "utils.h"
#include "settings.h"
#include "details_widget.h"
#include "display_specifier.h"

#include <QTreeView>
#include <QIcon>
#include <QSet>
#include <QStack>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QDebug>

enum ContainersColumn {
    ContainersColumn_Name,
    ContainersColumn_DN,
    ContainersColumn_COUNT,
};

QStandardItem *make_row(QStandardItem *parent, const QString &dn);

ContainersWidget::ContainersWidget(QWidget *parent)
: QWidget(parent)
{
    model = new ContainersModel(this);
    const auto advanced_view_proxy = new AdvancedViewProxy(ContainersColumn_DN, this);

    view = new QTreeView(this);
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setRootIsDecorated(true);
    view->setItemsExpandable(true);
    view->setExpandsOnDoubleClick(true);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setAllColumnsShowFocus(true);
    ObjectContextMenu::connect_view(view, ContainersColumn_DN);

    setup_model_chain(view, model, {advanced_view_proxy});
    
    setup_column_toggle_menu(view, model, {ContainersColumn_Name});

    get_containers_filter_classes();
    
    // Insert label into layout
    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);

    connect(
        AdInterface::instance(), &AdInterface::modified,
        this, &ContainersWidget::reload);
    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &ContainersWidget::on_selection_changed);

    connect(
        view, &QAbstractItemView::clicked,
        this, &ContainersWidget::on_view_clicked);

    const BoolSettingSignal *show_non_containers_signal = Settings::instance()->get_bool_signal(BoolSetting_ShowNonContainersInContainersTree);
    connect(
        show_non_containers_signal, &BoolSettingSignal::changed,
        this, &ContainersWidget::reload);
};

void ContainersWidget::reload() {
    const QString head_dn = AdInterface::instance()->get_search_base();
    QAbstractItemModel *view_model = view->model();

    // Save DN's that were fetched
    QSet<QString> fetched;
    {
        QStack<QModelIndex> stack;
        const QModelIndex head = model->index(0, 0);
        stack.push(head);

        while (!stack.isEmpty()) {
            const QModelIndex index = stack.pop();
            const bool index_fetched = !model->canFetchMore(index);

            if (index_fetched) {
                const QString dn = get_dn_from_index(index, ContainersColumn_DN);
                fetched.insert(dn);

                int row = 0;
                while (true) {
                    const QModelIndex child = model->index(row, 0, index);

                    if (child.isValid()) {
                        stack.push(child);
                        row++;
                    } else {
                        break;
                    }
                }
            }
        }
    }

    // Save DN's that were expanded in view
    QSet<QString> expanded;
    {
        QStack<QModelIndex> stack;
        const QModelIndex head = view_model->index(0, 0);
        stack.push(head);

        while (!stack.isEmpty()) {
            const QModelIndex index = stack.pop();

            if (view->isExpanded(index)) {
                const QString dn = get_dn_from_index(index, ContainersColumn_DN);
                expanded.insert(dn);

                int row = 0;
                while (true) {
                    const QModelIndex child = view_model->index(row, 0, index);

                    if (child.isValid()) {
                        stack.push(child);
                        row++;
                    } else {
                        view->expand(index);
                        break;
                    }
                }
            }
        }
    }

    // Clear model
    model->removeRows(0, model->rowCount());

    // NOTE: objects with changed DN's won't be refetched/re-expanded

    // Reload model, refetching everything that was fetched before clear
    {
        QStack<QModelIndex> stack;

        QStandardItem *invis_root = model->invisibleRootItem();
        const QStandardItem *head_item = make_row(invis_root, head_dn);
        stack.push(head_item->index());

        while (!stack.isEmpty()) {
            const QModelIndex index = stack.pop();
            const QString dn = get_dn_from_index(index, ContainersColumn_DN);
            const bool was_fetched = fetched.contains(dn);

            if (was_fetched) {
                model->fetchMore(index);

                int row = 0;
                while (true) {
                    const QModelIndex child = model->index(row, 0, index);

                    if (child.isValid()) {
                        stack.push(child);
                        row++;
                    } else {
                        view->expand(index);
                        break;
                    }
                }
            }
        }
    }

    // Re-expand items in view
    {
        const QModelIndex head = view_model->index(0, 0);
        QStack<QModelIndex> stack;
        stack.push(head);

        while (!stack.isEmpty()) {
            const QModelIndex index = stack.pop();
            const QString dn = get_dn_from_index(index, ContainersColumn_DN);
            const bool expand_index = expanded.contains(dn);

            if (expand_index) {
                view->expand(index);

                int row = 0;
                while (true) {
                    const QModelIndex child = view_model->index(row, 0, index);

                    if (child.isValid()) {
                        stack.push(child);
                        row++;
                    } else {
                        view->expand(index);
                        break;
                    }
                }
            }
        }
    }
}

void ContainersWidget::on_selection_changed(const QItemSelection &selected, const QItemSelection &) {
    // Transform selected index into source index and pass it on
    // to selected_container_changed() signal
    const QList<QModelIndex> indexes = selected.indexes();
    if (indexes.isEmpty()) {
        return;
    }

    const QString dn = get_dn_from_index(indexes[0], ContainersColumn_DN);

    emit selected_changed(dn);
}

void ContainersWidget::on_view_clicked(const QModelIndex &index) {
    const bool details_from_containers = Settings::instance()->get_bool(BoolSetting_DetailsFromContainers);

    if (details_from_containers) {
        const QString dn = get_dn_from_index(index, ContainersColumn_DN);
        DetailsWidget::change_target(dn);
    }
}

ContainersModel::ContainersModel(QObject *parent)
: ObjectModel(ContainersColumn_COUNT, ContainersColumn_DN, parent)
{
    set_horizontal_header_labels_from_map(this, {
        {ContainersColumn_Name, tr("Name")},
        {ContainersColumn_DN, tr("DN")}
    });

    // Load head
    const QString head_dn = AdInterface::instance()->get_search_base();
    QStandardItem *invis_root = invisibleRootItem();
    make_row(invis_root, head_dn);
}

bool ContainersModel::canFetchMore(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return false;
    }

    bool can_fetch = parent.data(ContainersModel::Roles::CanFetch).toBool();

    return can_fetch;
}

void ContainersModel::fetchMore(const QModelIndex &parent) {
    if (!parent.isValid() || !canFetchMore(parent)) {
        return;
    }

    QString dn = get_dn_from_index(parent, ContainersColumn_DN);

    QStandardItem *parent_item = itemFromIndex(parent);

    // Add children
    QList<QString> children = AdInterface::instance()->list(dn);

    for (auto child : children) {
        make_row(parent_item, child);
    }

    // Unset CanFetch flag
    parent_item->setData(false, ContainersModel::Roles::CanFetch);
}

// Override this so that unexpanded and unfetched items show the expander even though they technically don't have any children loaded
// NOTE: expander is show if hasChildren returns true
bool ContainersModel::hasChildren(const QModelIndex &parent = QModelIndex()) const {
    if (canFetchMore(parent)) {
        return true;
    } else {
        return QStandardItemModel::hasChildren(parent);
    }
}

// Make row in model at given parent based on object with given dn
QStandardItem *make_row(QStandardItem *parent, const QString &dn) {
    const bool passes_filter =
    [dn]() {
        static const QList<QString> filter_classes = get_containers_filter_classes();

        for (const auto acceptable_class : filter_classes) {
            const bool is_class = AdInterface::instance()->is_class(dn, acceptable_class);

            if (is_class) {
                return true;
            }
        }

        return false;
    }();

    const auto classes = AdInterface::instance()->attribute_get_multi(dn, ATTRIBUTE_OBJECT_CLASS);

    const bool ignore_filter = Settings::instance()->get_bool(BoolSetting_ShowNonContainersInContainersTree);
    if (!passes_filter && !ignore_filter) {
        return nullptr;
    }

    const QList<QStandardItem *> row = make_item_row(ContainersColumn_COUNT);
    
    const QString name = AdInterface::instance()->attribute_get(dn, "name");
    row[ContainersColumn_Name]->setText(name);
    row[ContainersColumn_DN]->setText(dn);

    const QIcon icon = get_object_icon(dn);
    row[0]->setIcon(icon);

    // Can fetch(expand) object if it's a container
    row[0]->setData(passes_filter, ContainersModel::Roles::CanFetch);

    parent->appendRow(row);

    return row[0];
}
