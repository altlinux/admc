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
#include "ad_object.h"
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

    proxy = new ContainersFilterProxy(this);

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
    view->sortByColumn(ContainersColumn_Name, Qt::AscendingOrder);

    proxy->setSourceModel(model);
    view->setModel(proxy);

    setup_column_toggle_menu(view, model, {ContainersColumn_Name});

    // Insert label into layout
    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);

    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &ContainersWidget::on_selection_changed);

    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &ContainersWidget::on_context_menu);

    // Make row for head object
    QStandardItem *invis_root = model->invisibleRootItem();
    const QString head_dn = AD()->domain_head();
    const AdObject head_object = AD()->search_object(head_dn);

    make_row(invis_root, head_object);
};

// Transform selected index into source index and pass it on
// to selected_container_changed() signal
void ContainersWidget::on_selection_changed(const QItemSelection &selected, const QItemSelection &) {
    const QList<QModelIndex> indexes = selected.indexes();
    if (indexes.isEmpty()) {
        return;
    }

    // Fetch selected object to remove expander if it has not children
    const QModelIndex index_proxy = indexes[0];
    const QModelIndex index = proxy->mapToSource(index_proxy);
    model->fetchMore(index);

    const QString dn = get_dn_from_index(index, ContainersColumn_DN);

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

void ContainersWidget::showEvent(QShowEvent *event) {
    resize_columns(view,
    {
        {ContainersColumn_Name, 0.5},
        {ContainersColumn_DN, 0.5},
    });
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

    const QString parent_dn = get_dn_from_index(parent, ContainersColumn_DN);

    QStandardItem *parent_item = itemFromIndex(parent);

    // Add children
    const QList<QString> search_attributes = {ATTRIBUTE_NAME, ATTRIBUTE_DISTINGUISHED_NAME, ATTRIBUTE_OBJECT_CLASS};
    const QString filter = current_advanced_view_filter();
    const QHash<QString, AdObject> search_results = AD()->search(filter, search_attributes, SearchScope_Children, parent_dn);

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

        if (parent_dn == search_base) {
            load_manually(configuration_dn);
        } else if (parent_dn == configuration_dn) {
            load_manually(schema_dn);
        }
    }
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
    const bool is_container =
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

    const QList<QStandardItem *> row = make_item_row(ContainersColumn_COUNT);
    
    const QString name = object.get_string(ATTRIBUTE_NAME);
    const QString dn = object.get_dn();
    row[ContainersColumn_Name]->setText(name);
    row[ContainersColumn_DN]->setText(dn);

    const QIcon icon = object.get_icon();
    row[0]->setIcon(icon);

    // Can fetch(expand) object if it's a container
    row[0]->setData(is_container, ContainersModel::Roles::CanFetch);
    row[0]->setData(is_container, ContainersModel::Roles::IsContainer);

    parent->appendRow(row);

    return row[0];
}

ContainersFilterProxy::ContainersFilterProxy(QObject *parent)
: QSortFilterProxyModel(parent) {
    const BoolSettingSignal *show_non_containers_signal = SETTINGS()->get_bool_signal(BoolSetting_ShowNonContainersInContainersTree);
    connect(
        show_non_containers_signal, &BoolSettingSignal::changed,
        this, &ContainersFilterProxy::on_show_non_containers);
    on_show_non_containers();
}

void ContainersFilterProxy::on_show_non_containers() {
    // NOTE: get the setting here and save it instead of in
    // filterAcceptsRow() for better perfomance
    show_non_containers = SETTINGS()->get_bool(BoolSetting_ShowNonContainersInContainersTree);
    invalidateFilter();
}

bool ContainersFilterProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    if (show_non_containers) {
        return true;
    } else {
        const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        const bool is_container = index.data(ContainersModel::Roles::IsContainer).toBool();

        return is_container;
    }
}
