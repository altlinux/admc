
#include "contents_view.h"
#include "ad_model.h"

#include <QItemSelection>
#include <QSortFilterProxyModel>

// TODO: bake this assumptions into view classes
// Both contents and containers share the same source model, but have different proxy's to it
// So need to map from containers proxy to source then back to proxy of contents
void ContentsView::set_root_index_from_selection(const QItemSelection &selected, const QItemSelection &) {
    const QList<QModelIndex> indexes = selected.indexes();

    if (indexes.size() == 0) {
        return;
    }

    // Map from proxy model of given index to source model of this view (if needed)
    QModelIndex source_index = indexes[0];
    {
        auto proxy_model = qobject_cast<const QSortFilterProxyModel *>(source_index.model());
        if (proxy_model != nullptr) {
            source_index = proxy_model->mapToSource(source_index);
        }
    }

    // Map from source model of this view to proxy model of this view (if needed)
    QModelIndex contents_index = source_index;
    {
        auto proxy_model = qobject_cast<const QSortFilterProxyModel *>(this->model());
        if (proxy_model != nullptr) {
            contents_index = proxy_model->mapFromSource(contents_index);
        }
    }

    if (!this->model()->checkIndex(contents_index)) {
        printf("ContentsView::set_root_index_from_selection received bad index!\n");
        return;
    }

    this->setRootIndex(contents_index);

    // NOTE: have to hide columns after model update
    this->hideColumn(AdModel::Column::DN);
}
