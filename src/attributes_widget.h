
#ifndef ATTRIBUTES_WIDGET_H
#define ATTRIBUTES_WIDGET_H

#include <QWidget>

class QTreeView;
class QString;
class AttributesModel;

// Shows names and values of attributes of the entry selected in contents view
class AttributesWidget final : public QWidget {
Q_OBJECT

public:
    AttributesWidget();

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
};

#endif /* ATTRIBUTES_WIDGET_H */
