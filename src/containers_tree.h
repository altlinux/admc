
#ifndef CONTAINERS_VIEW_H
#define CONTAINERS_VIEW_H

#include "ad_filter.h"

#include <QObject>

class QTreeView;
class AdModel;
class QAction;
class QModelIndex;

// Shows names of AdModel as a tree
class ContainersTree : public QObject {
Q_OBJECT

public:
    ContainersTree(QTreeView *view, AdModel *model, QAction *advanced_view_toggle);

signals:
    void selected_container_changed(const QModelIndex &selected);

private slots:
    void on_selection_changed(const QItemSelection &selected, const QItemSelection &);

private:
    QTreeView *view;
    AdFilter proxy;

};

#endif /* CONTAINERS_VIEW_H */
