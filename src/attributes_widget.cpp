
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

    view = new QTreeView(this);
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

void AttributesWidget::change_model_target(const QString &new_target_dn) {
    // Set model to new target
    model->change_target(new_target_dn);
}
