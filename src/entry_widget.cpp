
#include "entry_widget.h"
#include "ad_interface.h"
#include "ad_model.h"
#include "ad_filter.h"

#include <QApplication>
#include <QItemSelection>
#include <QSortFilterProxyModel>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QTreeView>
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>

EntryWidget::EntryWidget(AdModel* model, QAction *advanced_view_toggle)
: QWidget()
{
    proxy = new AdFilter(model, advanced_view_toggle);
    
    view = new QTreeView();
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setModel(proxy);

    label = new QLabel("LABEL");

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);
    layout()->addWidget(label);
    layout()->addWidget(view);

    // Init column visibility
    for (int column_i = AdModel::Column::Name; column_i < AdModel::Column::COUNT; column_i++) {
        auto column = static_cast<AdModel::Column>(column_i);

        column_hidden[column] = false;
    }
    update_column_visibility();

    // Convert view's customContextMenuRequested
    // to context_menu_requested signal with global pos
    connect(
        view, &QWidget::customContextMenuRequested,
        [this] (const QPoint &pos) {
            QModelIndex index = view->indexAt(pos);

            if (index.isValid()) {
                QPoint global_pos = view->mapToGlobal(pos);
                
                emit context_menu_requested(global_pos);
            }
        });
}

QString EntryWidget::get_selected_dn() const {
    // Return dn of selected entry, if any is selected and view
    // has focus
    const auto selection_model = view->selectionModel();

    if (view->hasFocus() && selection_model->hasSelection()) {
        auto selected_indexes = selection_model->selectedIndexes();
        auto selected = selected_indexes[0];
        QModelIndex dn_index = selected.siblingAtColumn(AdModel::Column::DN);

        return dn_index.data().toString();
    } else {
        return "";
    }
}

void EntryWidget::on_action_toggle_dn(bool checked) {
    const bool dn_column_hidden = !checked;
    column_hidden[AdModel::Column::DN] = dn_column_hidden;

    update_column_visibility();
}

void EntryWidget::update_column_visibility() {
    // Set column visiblity to current values in column_hidden
    for (int column_i = AdModel::Column::Name; column_i < AdModel::Column::COUNT; column_i++) {
        auto column = static_cast<AdModel::Column>(column_i);

        view->setColumnHidden(column, column_hidden[column]);
    }
}
