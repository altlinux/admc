
#ifndef CONTAINERS_WIDGET_H
#define CONTAINERS_WIDGET_H

#include "entry_widget.h"

class QItemSelection;

// Shows names of AdModel as a tree
class ContainersWidget final : public EntryWidget {
Q_OBJECT

public:
    ContainersWidget(AdModel *model);

signals:
    void selected_container_changed(const QModelIndex &selected);

private slots:
    void on_selection_changed(const QItemSelection &selected, const QItemSelection &);

private:

};

#endif /* CONTAINERS_WIDGET_H */
