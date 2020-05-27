
#include "attributes_model.h"
#include "ad_model.h"
#include "ad_interface.h"

AttributesModel::AttributesModel(QObject *parent)
: QStandardItemModel(0, Column::COUNT, parent)
{
    change_target(QString(""));

    QObject::connect(
        &ad_interface, &AdInterface::entry_deleted,
        this, &AttributesModel::on_entry_deleted);
}

// This will be called when an attribute value is edited
bool AttributesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    QModelIndex value_index = index;
    QModelIndex name_index = value_index.siblingAtColumn(AttributesModel::Column::Name);

    const QString attribute = name_index.data().toString();
    const QString value_str = value.toString();
    // printf("setData: %s, %s, %s\n", qPrintable(target_dn), qPrintable(attribute), qPrintable(value_str));

    bool success = set_attribute(target_dn, attribute, value_str);

    if (success) {
        QStandardItemModel::setData(index, value, role);

        return true;
    } else {
        return false;
    }
}

void AttributesModel::change_target(const QString &new_target_dn) {
    target_dn = new_target_dn;

    // Clear old data
    clear();
    
    // NOTE: need to reset headers after clearing
    setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    setHorizontalHeaderItem(Column::Value, new QStandardItem("Value"));

     // Populate model with attributes of new root
    QMap<QString, QList<QString>> attributes = get_attributes(target_dn);
    for (auto attribute : attributes.keys()) {
        QList<QString> values = attributes[attribute];

        for (auto value : values) {
            auto name_item = new QStandardItem(attribute);
            auto value_item = new QStandardItem(value);

            name_item->setEditable(false);

            appendRow({name_item, value_item});
        }
    }
}

void AttributesModel::on_entry_deleted(const QString &dn) {
    // Clear data if current target was deleted
    if (target_dn == dn) {
        change_target(QString(""));
    }
}