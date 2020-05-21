
#include "containers_tree.h"
#include "contents_list.h"
#include "attributes_list.h"
#include "ad_filter.h"
#include "ad_model.h"
#include "attributes_model.h"
#include "create_entry_dialog.h"
#include "ad_interface.h"
#include "entry_context_menu.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QString>

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
    Ui::MainWindow ui;
    QMainWindow main_window;
    ui.setupUi(&main_window);

    AdModel ad_model;

    // Attributes
    AttributesList attributes_view(ui.attributes_view);

    // Containers
    AdFilter containers_proxy(ui.menubar_view_advancedView);
    containers_proxy.setSourceModel(&ad_model);
    ContainersTree containers_tree(ui.containers_view, &containers_proxy);

    // Contents
    AdFilter contents_proxy(ui.menubar_view_advancedView);
    contents_proxy.setSourceModel(&ad_model);
    ContentsList contents_list(ui.contents_view, &containers_proxy);

    //
    // Entry context menu
    //
    {
        auto entry_context_menu = new EntryContextMenu(&main_window);

        // Popup entry context menu from both containers and contents views
        entry_context_menu->connect_view(*(ui.containers_view));
        entry_context_menu->connect_view(*(ui.contents_view));

        // Set target dn of attributes view when attributes menu of
        // entry context menu is clicked
        QObject::connect(
            entry_context_menu, &EntryContextMenu::attributes_clicked,
            &attributes_view, &AttributesList::set_target_dn);

        // Delete entry when delete button is pressed
        QObject::connect(
            entry_context_menu, &EntryContextMenu::delete_clicked,
            delete_entry);
    }

    // Connect signals to update models on when entries are modified
    QObject::connect(
        &ad_interface, &AdInterface::entry_deleted,
        &ad_model, &AdModel::on_entry_deleted);
    QObject::connect(
        &ad_interface, &AdInterface::entry_deleted,
        &attributes_view.model, &AttributesModel::on_entry_deleted);
    QObject::connect(
        &ad_interface, &AdInterface::entry_changed,
        &ad_model, &AdModel::on_entry_changed);
    QObject::connect(
        &ad_interface, &AdInterface::user_moved,
        &ad_model, &AdModel::on_user_moved);
    QObject::connect(
        &ad_interface, &AdInterface::entry_created,
        &ad_model, &AdModel::on_entry_created);

    // Set root index of contents view to selection of containers view
    QObject::connect(
        ui.containers_view->selectionModel(), &QItemSelectionModel::selectionChanged,
        &contents_list, &ContentsList::set_root_index_from_selection);
    
    // Connect menubar "New" submenu's to entry creation dialogs
    QObject::connect(
        ui.menubar_new_user, &QAction::triggered,
        create_user_dialog);
    QObject::connect(
        ui.menubar_new_computer, &QAction::triggered,
        create_computer_dialog);
    QObject::connect(
        ui.menubar_new_ou, &QAction::triggered,
        create_ou_dialog);
    QObject::connect(
        ui.menubar_new_group, &QAction::triggered,
        create_group_dialog);

    main_window.show();

    return app.exec();
}
