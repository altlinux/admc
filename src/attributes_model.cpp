
#include "attributes_model.h"
#include "ad_model.h"

AttributesModel::AttributesModel(): QStandardItemModel(0, Column::COUNT) {
    this->setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    this->setHorizontalHeaderItem(Column::Value, new QStandardItem("Value"));
}

// This will be called when an attribute value is edited
bool AttributesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    // auto value_str = value.toString();

    auto dn = index.data(AdModel::Roles::DN).toString();
    printf("%s\n", qPrintable(dn));

    // bool result = set_attribute();

    // QStandardItemModel::setData(index, value, role);

    // TODO: attribute edit can fail for many reasons, handle it

    // TODO: reload attributes in ad model
    
    return false;
}
