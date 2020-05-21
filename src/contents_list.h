
#ifndef CONTENTS_VIEW_H
#define CONTENTS_VIEW_H

#include <QWidget>

class QTreeView;
class AdFilter;
class QItemSelection;

// Shows name, category and description of children of entry selected in containers view
class ContentsList : public QWidget {
Q_OBJECT

public:
    explicit ContentsList(QTreeView *view, AdFilter *proxy);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

public slots:
    void set_root_index_from_selection(const QItemSelection &selected, const QItemSelection &);

private:
    QPoint drag_start_position;
    QTreeView *view;
};

#endif /* CONTENTS_VIEW_H */
