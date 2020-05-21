
#include "attributes_list.h"
#include "ad_interface.h"

#include <QWidget>
#include <QTreeView>

AttributesList::AttributesList(QTreeView *view): QObject() {
    this->view = view;

    view->setModel(&model);
};

void AttributesList::set_target_dn(const QString &new_target_dn) {
    this->target_dn = new_target_dn;

    // Clear model of previous root
    model.change_target(this->target_dn);

    // Populate model with attributes of new root
    QMap<QString, QList<QString>> attributes = load_attributes(this->target_dn);
    for (auto attribute : attributes.keys()) {
        QList<QString> values = attributes[attribute];

        for (auto value : values) {
            auto name_item = new QStandardItem(attribute);
            auto value_item = new QStandardItem(value);

            name_item->setEditable(false);

            model.appendRow({name_item, value_item});
        }
    }
}
