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
#include "object_context_menu.h"
#include "utils.h"
#include "settings.h"
#include "details_dialog.h"
#include "ad_config.h"
#include "ad_interface.h"
#include "filter.h"

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

QStandardItem *make_row(QStandardItem *parent, const AdObject &object);

ContainersWidget::ContainersWidget(QWidget *parent)
: QWidget(parent)
{
    model = new ContainersModel(this);

    view = new QTreeView(this);
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setRootIsDecorated(true);
    view->setItemsExpandable(true);
    view->setExpandsOnDoubleClick(true);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);

    view->setModel(model);

    setup_column_toggle_menu(view, model, {ContainersColumn_Name});

    // Insert label into layout
    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);

    connect(
        AD(), &AdInterface::modified,
        this, &ContainersWidget::reload);
    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &ContainersWidget::on_selection_changed);

    const BoolSettingSignal *show_non_containers_signal = SETTINGS()->get_bool_signal(BoolSetting_ShowNonContainersInContainersTree);
    connect(
        show_non_containers_signal, &BoolSettingSignal::changed,
        this, &ContainersWidget::reload);

    const BoolSettingSignal *dev_mode_signal = SETTINGS()->get_bool_signal(BoolSetting_DevMode);
    connect(
        dev_mode_signal, &BoolSettingSignal::changed,
        this, &ContainersWidget::reload);

    const BoolSettingSignal *advanced_view_setting = SETTINGS()->get_bool_signal(BoolSetting_AdvancedView);
    connect(
        advanced_view_setting, &BoolSettingSignal::changed,
        this, &ContainersWidget::reload);

    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &ContainersWidget::on_context_menu);

    reload();
};

void ContainersWidget::reload() {
    const QString head_dn = AD()->domain_head();
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
                        break;
                    }
                }
            }
        }
    }

    // Clear model
    model->removeRows(0, model->rowCount());

    // NOTE: objects with changed DN's won't be refetched/re-expanded

    // Reload model, restoring fetch and expand states
    {
        QStack<QModelIndex> stack;

        QStandardItem *invis_root = model->invisibleRootItem();
        const AdObject head_object = AD()->search_object(head_dn);
        const QStandardItem *head_item = make_row(invis_root, head_object);
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
                        break;
                    }
                }
            }

            const bool was_expanded = expanded.contains(dn);
            if (was_expanded) {
                view->expand(index);
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

    // Fetch selected object to remove expander if it has not children
    model->fetchMore(indexes[0]);

    const QString dn = get_dn_from_index(indexes[0], ContainersColumn_DN);

    emit selected_changed(dn);
}

void ContainersWidget::on_context_menu(const QPoint pos) {
    const QString dn = get_dn_from_pos(pos, view, ContainersColumn_DN);
    if (dn.isEmpty()) {
        return;
    }

    ObjectContextMenu context_menu(dn, view);
    exec_menu_from_view(&context_menu, view, pos);
}

ContainersModel::ContainersModel(QObject *parent)
: ObjectModel(ContainersColumn_COUNT, ContainersColumn_DN, parent)
{
    set_horizontal_header_labels_from_map(this, {
        {ContainersColumn_Name, tr("Name")},
        {ContainersColumn_DN, tr("DN")}
    });
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

    const QString dn = get_dn_from_index(parent, ContainersColumn_DN);

    QStandardItem *parent_item = itemFromIndex(parent);

    // Add children
    const QList<QString> search_attributes = {ATTRIBUTE_NAME, ATTRIBUTE_DISTINGUISHED_NAME, ATTRIBUTE_OBJECT_CLASS};
    const QString filter = current_advanced_view_filter();
    const QHash<QString, AdObject> search_results = AD()->search(filter, search_attributes, SearchScope_Children, dn);

    for (const AdObject object : search_results.values()) {
        make_row(parent_item, object);
    }

    // Unset CanFetch flag
    parent_item->setData(false, ContainersModel::Roles::CanFetch);

    // In dev mode, load configuration and schema objects
    // NOTE: have to manually add configuration and schema objects because they aren't searchable
    const bool dev_mode = SETTINGS()->get_bool(BoolSetting_DevMode);
    if (dev_mode) {
        const QString search_base = AD()->domain_head();
        const QString configuration_dn = AD()->configuration_dn();
        const QString schema_dn = AD()->schema_dn();

        const auto load_manually =
        [parent_item](const QString &child) {
            const AdObject object = AD()->search_object(child);
            make_row(parent_item, object);
        };

        if (dn == search_base) {
            load_manually(configuration_dn);
        } else if (dn == configuration_dn) {
            load_manually(schema_dn);
        }
    }

    sort(ContainersColumn_Name);
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
QStandardItem *make_row(QStandardItem *parent, const AdObject &object) {
    const bool passes_filter =
    [object]() {
        static const QList<QString> accepted_classes =
        []() {
            QList<QString> out = ADCONFIG()->get_filter_containers();

            // Make configuration and schema pass filter in dev mode so they are visible and can be fetched
            const bool dev_mode = SETTINGS()->get_bool(BoolSetting_DevMode);
            if (dev_mode) {
                out.append({CLASS_CONFIGURATION, CLASS_dMD});
            }

            // TODO: domain not included for some reason, so add it ourselves
            out.append(CLASS_DOMAIN);
            
            return out;
        }();

        // NOTE: compare against all classes of object, not just the most derived one
        const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
        for (const auto acceptable_class : accepted_classes) {
            if (object_classes.contains(acceptable_class)) {
                return true;
            }
        }

        return false;
    }();

    const bool ignore_filter = SETTINGS()->get_bool(BoolSetting_ShowNonContainersInContainersTree);
    if (!passes_filter && !ignore_filter) {
        return nullptr;
    }

    const QList<QStandardItem *> row = make_item_row(ContainersColumn_COUNT);
    
    const QString name = object.get_string(ATTRIBUTE_NAME);
    const QString dn = object.get_dn();
    row[ContainersColumn_Name]->setText(name);
    row[ContainersColumn_DN]->setText(dn);

    const QIcon icon = object.get_icon();
    row[0]->setIcon(icon);

    // Can fetch(expand) object if it's a container
    row[0]->setData(passes_filter, ContainersModel::Roles::CanFetch);

    parent->appendRow(row);

    return row[0];
}
