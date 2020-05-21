
#include "containers_tree.h"
#include "ad_model.h"

#include <QTreeView>

ContainersTree::ContainersTree(QTreeView *view, AdModel *model, QAction *advanced_view_toggle):
proxy(model, advanced_view_toggle) 
{
    this->view = view;

    proxy.only_show_containers = true;

    view->setModel(&proxy);
    view->hideColumn(AdModel::Column::Category);
    view->hideColumn(AdModel::Column::Description);
    view->hideColumn(AdModel::Column::DN);

    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &ContainersTree::on_selection_changed);
};

void ContainersTree::on_selection_changed(const QItemSelection &selected, const QItemSelection &) {
    // Transform selected index into source index and pass it on
    // to selected_container_changed() signal
    const QList<QModelIndex> indexes = selected.indexes();

    if (indexes.size() > 0) {
        QModelIndex index = indexes[0];
        QModelIndex source_index = proxy.mapToSource(index);

        emit selected_container_changed(source_index);
    }
}

