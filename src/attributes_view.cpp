
#include "attributes_view.h"
#include "attributes_model.h"
#include "ad_model.h"
#include "ad_interface.h"

// TODO: on attribute edit, update entry in ad model
// since attributes affect model's contents

void AttributesView::set_target_from_selection(const QItemSelection &selected, const QItemSelection &) {
    // Convert selection to dn
    auto indexes = selected.indexes();
    if (indexes.size() == 0) {
        return;
    }

    auto index = indexes[0];
    this->target_dn = index.data(AdModel::Roles::DN).toString();

    // Clear model of previous root
    // TODO: get rid of cast
    auto model = qobject_cast<AttributesModel *>(this->model());
    if (model != nullptr) {
        model->change_target(this->target_dn);
    }

    // Populate model with attributes of new root
    auto attributes = load_attributes(this->target_dn);
    for (auto attribute : attributes.keys()) {
        auto values = attributes[attribute];

        for (auto value : values) {
            auto name_item = new QStandardItem(attribute);
            auto value_item = new QStandardItem(value);

            name_item->setEditable(false);

            model->appendRow({name_item, value_item});
        }
    }
}
