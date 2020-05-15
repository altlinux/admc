
#include "attributes_model.h"
#include "ad_model.h"
#include "ad_interface.h"

AttributesModel::AttributesModel(): QStandardItemModel(0, Column::COUNT) {
    auto f = QString("");
    change_target(f);
}

// This will be called when an attribute value is edited
bool AttributesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    auto value_index = index;
    auto name_index = value_index.siblingAtColumn(AttributesModel::Column::Name);

    auto dn = this->target_dn;
    auto attribute = name_index.data().toString();
    auto value_str = value.toString();
    // printf("setData: %s, %s, %s\n", qPrintable(dn), qPrintable(attribute), qPrintable(value_str));

    // TODO: attribute edit can fail for many reasons, handle it
    bool success = set_attribute(dn, attribute, value_str);

    if (success) {
        QStandardItemModel::setData(index, value, role);

        emit entry_changed(dn);

        return true;
    } else {
        return false;
    }
}

void AttributesModel::change_target(QString &new_target_dn) {
    this->target_dn = new_target_dn;

    // Clear old data
    // NOTE: need to reset headers after clearing
    this->clear();
    this->setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    this->setHorizontalHeaderItem(Column::Value, new QStandardItem("Value"));
}
