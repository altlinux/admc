
#ifndef CONTAINERS_VIEW_H
#define CONTAINERS_VIEW_H

class QTreeView;
class AdFilter;

// Shows names of AdModel as a tree
class ContainersView {

public:
    ContainersView(QTreeView *view, AdFilter *proxy);

private:
    QTreeView *view;

};

#endif /* CONTAINERS_VIEW_H */
