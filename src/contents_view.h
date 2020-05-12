
#pragma once

#include <QTreeView>

// Shows name, category and description of children of entry selected in containers view
class ContentsView : public QTreeView {
Q_OBJECT

public:
    using QTreeView::QTreeView;

public slots:
    void set_root_index_from_selection(const QItemSelection &selected, const QItemSelection &);

private:

};