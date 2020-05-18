
#include "attributes_view.h"
#include "attributes_model.h"
#include "ad_model.h"
#include "ad_interface.h"

// TODO: on attribute edit, update entry in ad model
// since attributes affect model's contents

void AttributesView::set_target_dn(const QString &new_target_dn) {
    this->target_dn = new_target_dn;

    // Clear model of previous root
    // TODO: get rid of cast
    AttributesModel *model = qobject_cast<AttributesModel *>(this->model());
    if (model != nullptr) {
        model->change_target(this->target_dn);
    }

    // Populate model with attributes of new root
    QMap<QString, QList<QString>> attributes = load_attributes(this->target_dn);
    for (auto attribute : attributes.keys()) {
        QList<QString> values = attributes[attribute];

        for (auto value : values) {
            auto name_item = new QStandardItem(attribute);
            auto value_item = new QStandardItem(value);

            name_item->setEditable(false);

            model->appendRow({name_item, value_item});
        }
    }
}
