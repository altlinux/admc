#pragma once

#include <QStandardItemModel>

class AdModel: public QStandardItemModel {
Q_OBJECT

public:
    enum Column {
        Name,
        Category,
        Description,
        COUNT,
    };

    explicit AdModel();

    enum Roles {
        DN = Qt::UserRole + 1,
        AdvancedViewOnly = Qt::UserRole + 2,
        CanFetch = Qt::UserRole + 3,
        IsContainer = Qt::UserRole + 4,
    };

    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);
    bool hasChildren(const QModelIndex &parent) const override;

private:

};
