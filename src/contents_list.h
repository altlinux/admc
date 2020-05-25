
#ifndef CONTENTS_VIEW_H
#define CONTENTS_VIEW_H

#include "ad_filter.h"

#include <QObject>

class QTreeView;
class AdModel;
class QAction;
class QItemSelection;

// Shows name, category and description of children of entry selected in containers view
class ContentsList : public QObject {
Q_OBJECT

public:
    ContentsList(QTreeView *view, AdModel *model, QAction *advanced_view);

    bool dn_column_hidden = true;

    void update_column_visibility();

public slots:
    void on_selected_container_changed(const QModelIndex &source_index);

private:
    QTreeView *view;
    AdFilter proxy;
};

#endif /* CONTENTS_VIEW_H */
