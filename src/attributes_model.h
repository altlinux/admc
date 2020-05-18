
#ifndef ATTRIBUTES_MODEL_H
#define ATTRIBUTES_MODEL_H

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
    void change_target(const QString &new_target_dn);

signals:
    void entry_changed(const QString &dn); 

public slots:
    void on_entry_deleted(const QString &dn); 

private:
    QString target_dn;

};

#endif /* ATTRIBUTES_MODEL_H */
