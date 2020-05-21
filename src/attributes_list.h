
#ifndef ATTRIBUTES_VIEW_H
#define ATTRIBUTES_VIEW_H

#include "attributes_model.h"

#include <QObject>

class QTreeView;
class QString;
class AttributesModel;

// Shows names and values of attributes of the entry selected in contents view
class AttributesList : public QObject {
Q_OBJECT

public:
    AttributesList(QTreeView *view);

    AttributesModel model;

public slots:
    void set_target_dn(const QString &new_target_dn);

private:
    enum Column {
        Name,
        Value,
        COUNT,
    };
    
    QTreeView *view;
    QString target_dn;
};

#endif /* ATTRIBUTES_VIEW_H */
