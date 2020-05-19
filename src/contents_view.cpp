
#include "contents_view.h"
#include "ad_interface.h"
#include "ad_model.h"

#include <QApplication>
#include <QItemSelection>
#include <QSortFilterProxyModel>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

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

// TODO: currently dragging doesn't work correctly most of the time
// the dragged item is not drawn (always the "X" icon)
// and on drag complete the item is not moved correctly
// icon is incorrect for example
// probably from dragging being started incorrectly
void ContentsView::mousePressEvent(QMouseEvent *event) {
    QTreeView::mousePressEvent(event);
    
    // Record drag position
    if (event->button() == Qt::LeftButton) {
        this->drag_start_position = event->pos();
    }
}

void ContentsView::mouseMoveEvent(QMouseEvent *event) {
    QTreeView::mouseMoveEvent(event);
    
    // Start drag event if holding left mouse button and dragged far enough

    bool holding_left_button = event->buttons() & Qt::LeftButton;
    if (!holding_left_button) {
        return;
    }

    int drag_distance = (event->pos() - this->drag_start_position).manhattanLength();
    if (drag_distance < QApplication::startDragDistance()) {
        return;
    }

    printf("drag start\n");

    QDrag *drag = new QDrag(this);

    // Set drag data to the DN of clicked entry
    QPoint pos = event->pos();
    QModelIndex index = this->indexAt(pos);
    QModelIndex dn_index = index.siblingAtColumn(AdModel::Column::DN);
    QString dn = dn_index.data().toString();
    QMimeData *mime_data = new QMimeData();
    mime_data->setText(dn);
    drag->setMimeData(mime_data);

    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
}

void ContentsView::dragEnterEvent(QDragEnterEvent *event) {
    QTreeView::dragEnterEvent(event);
    
    // TODO: is this needed?
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}

bool can_drop_at() {
    // TODO: write this
    // not sure if to start from point or index
    return true;
}

void ContentsView::dragMoveEvent(QDragMoveEvent *event) {
    // Determine whether drag action is accepted at currently 
    // hovered entry
    // This only changes the drag icon

    QTreeView::dragMoveEvent(event);
    
    QPoint pos = event->pos();
    QModelIndex index = this->indexAt(pos);
    QModelIndex category_index = index.siblingAtColumn(AdModel::Column::Category);
    QString category = category_index.data().toString();

    // TODO: currently using the shortened category
    // should use objectClass? so it needs to be cached alone or maybe all attributes need to be cached, whichever happens first 
    if (category == "Container" || category == "Organizational-Unit") {
        event->accept();
    } else {
        event->ignore();
    }
}

void ContentsView::dropEvent(QDropEvent *event) {
    QTreeView::dropEvent(event);
    // TODO: should accept? determining whether move succeeded is delayed until ad request is complete, so not sure how that works out
    // event->acceptProposedAction();

    printf("drop xd\n");

    QString user_dn = event->mimeData()->text();

    QPoint pos = event->pos();
    QModelIndex index = this->indexAt(pos);
    QModelIndex dn_index = index.siblingAtColumn(AdModel::Column::DN);
    QString container_dn = dn_index.data().toString();

    move_user(user_dn, container_dn);
}
