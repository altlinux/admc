
#ifndef ATTRIBUTES_VIEW_H
#define ATTRIBUTES_VIEW_H

#include <QWidget>

class QTreeView;
class QString;
class AttributesModel;

// Shows names and values of attributes of the entry selected in contents view
class AttributesList : public QWidget {
Q_OBJECT

public:
    AttributesList();


public slots:
    void set_target_dn(const QString &new_target_dn);

private:
    enum Column {
        Name,
        Value,
        COUNT,
    };

    AttributesModel *model = nullptr;
    QTreeView *view = nullptr;
    QString target_dn;
};

#endif /* ATTRIBUTES_VIEW_H */
