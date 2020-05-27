
#include "attributes_widget.h"
#include "attributes_model.h"
#include "ad_interface.h"

#include <QWidget>
#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>

AttributesWidget::AttributesWidget()
: QWidget()
{
    model = new AttributesModel(this);

    view = new QTreeView();
    view->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
    view->setSelectionMode(QAbstractItemView::NoSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setModel(model);

    const auto label = new QLabel("Attributes");

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);
    layout()->addWidget(label);
    layout()->addWidget(view);
};

void AttributesWidget::set_target_dn(const QString &new_target_dn) {
    this->target_dn = new_target_dn;

    // Clear model of previous root
    model->change_target(this->target_dn);

    // Populate model with attributes of new root
    QMap<QString, QList<QString>> attributes = get_attributes(this->target_dn);
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
