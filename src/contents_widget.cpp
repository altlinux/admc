
#include "contents_widget.h"
#include "ad_interface.h"
#include "ad_model.h"
#include "ad_proxy_model.h"

#include <QApplication>
#include <QItemSelection>
#include <QSortFilterProxyModel>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QTreeView>
#include <QLabel>

ContentsWidget::ContentsWidget(AdModel* model, QAction *advanced_view_toggle)
: EntryWidget(model, advanced_view_toggle)
{   
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setRootIsDecorated(false);
    view->setItemsExpandable(false);
    view->setExpandsOnDoubleClick(false);

    label->setText("Contents");

    column_hidden[AdModel::Column::DN] = true;
    update_column_visibility();
};

// Both contents and containers share the same source model, but have different proxy's to it
// So need to map from containers proxy to source then back to proxy of contents
void ContentsWidget::on_selected_container_changed(const QModelIndex &source_index) {
    QModelIndex index = proxy->mapFromSource(source_index);
    view->setRootIndex(index);

    // NOTE: have to hide columns after setRootIndex
    update_column_visibility();
}
