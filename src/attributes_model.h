
#ifndef ATTRIBUTES_MODEL_H
#define ATTRIBUTES_MODEL_H

#include <QStandardItemModel>

class AttributesModel final : public QStandardItemModel {
Q_OBJECT

public:
    enum Column {
        Name,
        Value,
        COUNT,
    };

    explicit AttributesModel(QObject *parent);

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    void change_target(const QString &new_target_dn);

private slots:
    void on_delete_entry_complete(const QString &dn); 
    void on_move_user_complete(const QString &user_dn, const QString &container_dn, const QString &new_dn); 
    void on_load_attributes_complete(const QString &dn);

private:
    QString target_dn;

};

#endif /* ATTRIBUTES_MODEL_H */
