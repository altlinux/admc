
#include "contents_list.h"
#include "ad_interface.h"
#include "ad_model.h"
#include "ad_filter.h"

#include <QApplication>
#include <QItemSelection>
#include <QSortFilterProxyModel>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QTreeView>

ContentsList::ContentsList(QTreeView *view, AdModel* model, QAction *advanced_view_toggle)
: EntryWidget(view, model, advanced_view_toggle)
{
    column_hidden[AdModel::Column::DN] = true;
    update_column_visibility();
};

// Both contents and containers share the same source model, but have different proxy's to it
// So need to map from containers proxy to source then back to proxy of contents
void ContentsList::on_selected_container_changed(const QModelIndex &source_index) {
    QModelIndex index = proxy->mapFromSource(source_index);
    view->setRootIndex(index);

    // NOTE: have to hide columns after setRootIndex
    update_column_visibility();
}
