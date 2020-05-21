
#include "containers_tree.h"
#include "ad_model.h"

#include <QTreeView>

ContainersTree::ContainersTree(QTreeView *view, AdModel *model, QAction *advanced_view_toggle):
proxy(model, advanced_view_toggle) 
{
    this->view = view;

    proxy.only_show_containers = true;

    view->setModel(&proxy);
    view->hideColumn(AdModel::Column::Category);
    view->hideColumn(AdModel::Column::Description);
    view->hideColumn(AdModel::Column::DN);
};
