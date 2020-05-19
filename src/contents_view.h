
#ifndef CONTENTS_VIEW_H
#define CONTENTS_VIEW_H

#include <QTreeView>

class QPoint;

// Shows name, category and description of children of entry selected in containers view
class ContentsView : public QTreeView {
Q_OBJECT

public:
    using QTreeView::QTreeView;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

public slots:
    void set_root_index_from_selection(const QItemSelection &selected, const QItemSelection &);

private:
    QPoint drag_start_position;
};

#endif /* CONTENTS_VIEW_H */
