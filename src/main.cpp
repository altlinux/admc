
#include "containers_tree.h"
#include "contents_list.h"
#include "attributes_list.h"
#include "ad_filter.h"
#include "ad_model.h"
#include "attributes_model.h"
#include "create_entry_dialog.h"
#include "ad_interface.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QString>
#include <QAction>

Ui::MainWindow ui;
AttributesList *attributes_list;
QAction *action_attributes;
QAction *action_delete_entry;
QList<QAction *> new_entry_actions;

QString get_selected_dn() {
    QTreeView *focus_view = nullptr;
    
    if (ui.containers_view->hasFocus()) {
        focus_view = ui.containers_view;
    } else if (ui.contents_view->hasFocus()) {
        focus_view = ui.contents_view;
    }

    QString dn = "";

    if (focus_view != nullptr) {
        auto selection_model = focus_view->selectionModel();
        auto selected_indexes = selection_model->selectedIndexes();

        if (selected_indexes.size() > 0) {
            auto selected = selected_indexes[0];
            QModelIndex dn_index = selected.siblingAtColumn(AdModel::Column::DN);

            dn = dn_index.data().toString();
        }
    }

    return dn;
}

void on_action_attributes() {
    auto dn = get_selected_dn();
    attributes_list->set_target_dn(dn);
}

void on_action_delete_entry() {
    auto dn = get_selected_dn();
    delete_entry(dn);
}

void on_action_new_entry(NewEntryType type) {
    QString dn = get_selected_dn();
    create_entry_dialog(type, dn);
}

void popup_entry_context_menu(const QPoint &pos) {
    QMenu menu;

    menu.addAction(action_attributes);
    menu.addAction(action_delete_entry);

    QMenu *submenu_new = menu.addMenu("New");
    for (auto action : new_entry_actions) {
        submenu_new->addAction(action);
    }

    menu.exec(pos, action_attributes);
}

void connect_view_to_entry_context_menu(const QTreeView &view) {
    QObject::connect(
        &view, &QWidget::customContextMenuRequested,
        [&view] (const QPoint &pos) {
            // Get DN of clicked entry
            QModelIndex index = view.indexAt(pos);

            if (index.isValid()) {
                QPoint global_pos = view.mapToGlobal(pos);
                popup_entry_context_menu(global_pos);
            }
        });
}

int main(int argc, char **argv) {
    // Load fake AD data if given "fake" argument
    // This also swaps all ad interface functions to their fake versions (including login)
    if (argc >= 1 && QString(argv[1]) == "fake") {
        FAKE_AD = true;
    }

    if (!ad_interface_login()) {
        return 1;
    }

    QApplication app(argc, argv);

    //
    // Setup ui
    //
    QMainWindow main_window;
    ui.setupUi(&main_window);

    AdModel ad_model;

    ContainersTree containers_tree(ui.containers_view, &ad_model, ui.action_advanced_view);
    ContentsList contents_list(ui.contents_view, &ad_model, ui.action_advanced_view);
    attributes_list = new AttributesList(ui.attributes_view);

    // Setup actions
    action_attributes = new QAction("Attributes");
    action_delete_entry = new QAction("Delete");
    QObject::connect(
        action_attributes, &QAction::triggered,
        on_action_attributes);
    QObject::connect(
        action_delete_entry, &QAction::triggered,
        on_action_delete_entry);

    // Setup "New X" actions
    for (int type_i = NewEntryType::User; type_i < NewEntryType::COUNT; type_i++) {
        NewEntryType type = static_cast<NewEntryType>(type_i);
        QString label = new_entry_type_to_string[type];
        QAction *action = new QAction(label);

        QObject::connect(action, &QAction::triggered, [type]() {
            create_entry_dialog(type);
        });
        
        new_entry_actions.push_back(action);
    }

    connect_view_to_entry_context_menu(*ui.containers_view);
    connect_view_to_entry_context_menu(*ui.contents_view);

    // Set root index of contents view to selection of containers view
    QObject::connect(
        &containers_tree, &ContainersTree::selected_container_changed,
        &contents_list, &ContentsList::on_selected_container_changed);
    
    // Add menubar actions
    for (auto a : new_entry_actions) {
        ui.menubar_new->addAction(a);
    }

    main_window.show();

    return app.exec();
}
