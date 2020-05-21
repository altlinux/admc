
#include "attributes_model.h"
#include "ad_model.h"
#include "ad_interface.h"

AttributesModel::AttributesModel(): QStandardItemModel(0, Column::COUNT) {
    change_target(QString(""));

    QObject::connect(
        &ad_interface, &AdInterface::entry_deleted,
        this, &AttributesModel::on_entry_deleted);
}

// This will be called when an attribute value is edited
bool AttributesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    QModelIndex value_index = index;
    QModelIndex name_index = value_index.siblingAtColumn(AttributesModel::Column::Name);

    const QString dn = this->target_dn;
    const QString attribute = name_index.data().toString();
    const QString value_str = value.toString();
    // printf("setData: %s, %s, %s\n", qPrintable(dn), qPrintable(attribute), qPrintable(value_str));

    // TODO: attribute edit can fail for many reasons, handle it
    bool success = set_attribute(dn, attribute, value_str);

    if (success) {
        QStandardItemModel::setData(index, value, role);

        return true;
    } else {
        return false;
    }
}

void AttributesModel::change_target(const QString &new_target_dn) {
    this->target_dn = new_target_dn;

    // Clear old data
    // NOTE: need to reset headers after clearing
    this->clear();
    this->setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    this->setHorizontalHeaderItem(Column::Value, new QStandardItem("Value"));
}

void AttributesModel::on_entry_deleted(const QString &dn) {
    // Clear data if current target was deleted
    if (this->target_dn == dn) {
        this->change_target(QString(""));
    }
}