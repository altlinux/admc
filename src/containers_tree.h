
#ifndef CONTAINERS_VIEW_H
#define CONTAINERS_VIEW_H

#include "entry_widget.h"

class QItemSelection;

// Shows names of AdModel as a tree
class ContainersTree : public EntryWidget {
Q_OBJECT

public:
    ContainersTree(AdModel *model, QAction *advanced_view_toggle);

signals:
    void selected_container_changed(const QModelIndex &selected);

private slots:
    void on_selection_changed(const QItemSelection &selected, const QItemSelection &);

private:

};

#endif /* CONTAINERS_VIEW_H */
