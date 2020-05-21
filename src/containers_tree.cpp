
#include "containers_tree.h"
#include "ad_filter.h"
#include "ad_model.h"

#include <QTreeView>

ContainersTree::ContainersTree(QTreeView *view, AdFilter *proxy) {
    this->view = view;

    view->setModel(proxy);
    view->hideColumn(AdModel::Column::Category);
    view->hideColumn(AdModel::Column::Description);
    view->hideColumn(AdModel::Column::DN);
};
