
#ifndef ATTRIBUTES_VIEW_H
#define ATTRIBUTES_VIEW_H

#include <QTreeView>

// Shows names and values of attributes of the entry selected in contents view
class AttributesView : public QTreeView {
Q_OBJECT

public:
    using QTreeView::QTreeView;

public slots:
    void set_target_dn(const QString &new_target_dn);

private:
    enum Column {
        Name,
        Value,
        COUNT,
    };
    
    QString target_dn;

};

#endif /* ATTRIBUTES_VIEW_H */
