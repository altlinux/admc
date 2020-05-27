
#include "containers_widget.h"
#include "ad_model.h"
#include "ad_proxy_model.h"

#include <QTreeView>
#include <QLabel>

ContainersWidget::ContainersWidget(AdModel *model)
: EntryWidget(model)
{
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setRootIsDecorated(true);
    view->setItemsExpandable(true);
    view->setExpandsOnDoubleClick(true);

    proxy->only_show_containers = true;

    column_hidden[AdModel::Column::Name] = false;
    column_hidden[AdModel::Column::Category] = true;
    column_hidden[AdModel::Column::Description] = true;
    column_hidden[AdModel::Column::DN] = true;
    update_column_visibility();

    label->setText("Containers");

    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &ContainersWidget::on_selection_changed);
};

void ContainersWidget::on_selection_changed(const QItemSelection &selected, const QItemSelection &) {
    // Transform selected index into source index and pass it on
    // to selected_container_changed() signal
    const QList<QModelIndex> indexes = selected.indexes();

    if (indexes.size() > 0) {
        QModelIndex index = indexes[0];
        QModelIndex source_index = proxy->mapToSource(index);

        emit selected_container_changed(source_index);
    }
}
