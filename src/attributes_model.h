#pragma once

#include <QStandardItemModel>

class AttributesModel: public QStandardItemModel {
Q_OBJECT

public:
    enum Column {
        Name,
        Value,
        COUNT,
    };

    explicit AttributesModel();

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

private:

};
