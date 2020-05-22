
#include "entry_context_menu.h"
#include "ad_model.h"
#include "create_entry_dialog.h"
#include "ad_interface.h"

#include <QPoint>
#include <QAction>
#include <QTreeView>

EntryContextMenu::EntryContextMenu(QWidget *parent) : QMenu(parent) {
    QAction *attributes_action = new QAction("Attributes", this);
    connect(attributes_action, &QAction::triggered, [this]() {
        emit attributes_clicked(this->target_dn);
    });
    this->addAction(attributes_action);

    QAction *delete_action = new QAction("Delete", this);
    connect(delete_action, &QAction::triggered, [this]() {
        emit delete_clicked(this->target_dn);
    });
    this->addAction(delete_action);

    QMenu *submenu_new = this->addMenu("New");
    // Create "New X" menu for each entry type
    for (int type_i = NewEntryType::User; type_i < NewEntryType::COUNT; type_i++) {
        NewEntryType type = static_cast<NewEntryType>(type_i);
        QString action_label = new_entry_type_to_string[type];
        QAction *action = new QAction(action_label, this);
        connect(action, &QAction::triggered, [this, type]() {
            create_entry_dialog(type, this->target_dn);
        });
        submenu_new->addAction(action);
    }
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

            if (index.isValid()) {
                QString dn = get_dn_of_index(index);

                this->target_dn = dn;
                this->popup(global_pos);
            }
        });
}
