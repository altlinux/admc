
#include "attributes_model.h"
#include "ad_model.h"
#include "ad_interface.h"

AttributesModel::AttributesModel(QObject *parent)
: QStandardItemModel(0, Column::COUNT, parent)
{
    change_target(QString(""));

    QObject::connect(
        &ad_interface, &AdInterface::delete_entry_complete,
        this, &AttributesModel::on_delete_entry_complete);
}

// This will be called when an attribute value is edited
bool AttributesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    QModelIndex value_index = index;
    QModelIndex name_index = value_index.siblingAtColumn(AttributesModel::Column::Name);

    const QString attribute = name_index.data().toString();
    const QString value_str = value.toString();

    bool success = set_attribute(target_dn, attribute, value_str);

    // TODO: probably should add on_load_attributes_complete slot
    // and setData in there, maybe even just reload whole model
    // on each attribute edit to be safe

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

void AttributesModel::on_delete_entry_complete(const QString &dn) {
    // Clear data if current target was deleted
    if (target_dn == dn) {
        change_target(QString(""));
    }
}
