
#ifndef CONTAINERS_VIEW_H
#define CONTAINERS_VIEW_H

#include <QTreeView>

// Shows names of AdModel as a tree
class ContainersView : public QTreeView {
Q_OBJECT

public:
    explicit ContainersView(QWidget *parent = nullptr);

public slots:

signals:

private:

};

#endif /* CONTAINERS_VIEW_H */
