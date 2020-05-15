
#ifndef AD_MODEL_H
#define AD_MODEL_H

#include <QStandardItemModel>

class AdModel: public QStandardItemModel {
Q_OBJECT

public:
    enum Column {
        Name,
        Category,
        Description,
        DN,
        COUNT,
    };

    explicit AdModel();

    enum Roles {
        AdvancedViewOnly = Qt::UserRole + 1,
        CanFetch = Qt::UserRole + 2,
        IsContainer = Qt::UserRole + 3,
    };

    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);
    bool hasChildren(const QModelIndex &parent) const override;

public slots:
    void on_entry_changed(QString &dn); 

private:

};

#endif /* AD_MODEL_H */
