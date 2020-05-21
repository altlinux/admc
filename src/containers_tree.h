
#ifndef CONTAINERS_VIEW_H
#define CONTAINERS_VIEW_H

#include "ad_filter.h"

class QTreeView;
class AdModel;
class QAction;

// Shows names of AdModel as a tree
class ContainersTree {

public:
    ContainersTree(QTreeView *view, AdModel *model, QAction *advanced_view_toggle);

private:
    QTreeView *view;
    AdFilter proxy;

};

#endif /* CONTAINERS_VIEW_H */
