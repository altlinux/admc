
#include "containers_view.h"
#include "contents_view.h"
#include "attributes_view.h"
#include "ad_filter.h"
#include "ad_model.h"
#include "attributes_model.h"
#include "ui_mainwindow.h"
#include "ad_interface.h"

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
    {
        auto attributes_model = new AttributesModel();
        ui.attributes_view->setModel(attributes_model);
    }

    // Contents
    {
        auto view = ui.contents_view;

        auto proxy = new AdFilter(ui.menubar_view_advancedView);
        proxy->setSourceModel(&ad_model);
        view->setModel(proxy);
    }

    // Containers
    {
        auto view = ui.containers_view;

        auto proxy = new AdFilter(ui.menubar_view_advancedView, true);
        proxy->setSourceModel(&ad_model);
        view->setModel(proxy);

        // NOTE: have to hide columns after setting model
        view->hideColumn(AdModel::Column::Category);
        view->hideColumn(AdModel::Column::Description);
    }

    //
    // Connect signals
    //

    // Set root index of contents view to selection of containers view
    QObject::connect(
        ui.containers_view->selectionModel(), &QItemSelectionModel::selectionChanged,
        ui.contents_view, &ContentsView::set_root_index_from_selection);
    
    // Set target of attributes view to selection of contents view
    QObject::connect(
        ui.contents_view->selectionModel(), &QItemSelectionModel::selectionChanged,
        ui.attributes_view, &AttributesView::set_target_from_selection);

    main_window.show();

    return app.exec();
}
