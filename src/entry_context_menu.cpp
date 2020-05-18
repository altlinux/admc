
#include "entry_context_menu.h"
#include "ad_model.h"
#include "ad_interface.h"

#include <QPoint>
#include <QAction>
#include <QTreeView>

EntryContextMenu::EntryContextMenu(QWidget *parent) : QMenu(parent) {
    QAction *attributes_action = new QAction("Attributes", this);
    connect(attributes_action, &QAction::triggered, [this]() {
        emit attributes_clicked(this->target_dn);
    });

    QAction *delete_action = new QAction("Delete", this);
    connect(delete_action, &QAction::triggered, [this]() {
        delete_entry(this->target_dn);
        emit delete_clicked(this->target_dn);
    });

    this->addAction(attributes_action);
    this->addAction(delete_action);
}

void EntryContextMenu::connect_view(const QTreeView &view) {
    // Open entry context menu from given view
    // Save dn of clicked entry of the view
    connect(
        &view, &QWidget::customContextMenuRequested,
        [this, &view] (const QPoint& pos) {
            QPoint global_pos = view.mapToGlobal(pos);

            // Get DN of clicked entry
            QModelIndex index = view.indexAt(pos);
            QModelIndex dn_index = index.siblingAtColumn(AdModel::Column::DN);
            QString dn = dn_index.data().toString();

            this->target_dn = dn;
            this->popup(global_pos);
    });
}
