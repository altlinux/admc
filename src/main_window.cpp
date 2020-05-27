
#include "main_window.h"
#include "containers_widget.h"
#include "contents_widget.h"
#include "attributes_widget.h"
#include "ad_model.h"
#include "attributes_model.h"
#include "create_entry_dialog.h"

#include <QString>
#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTreeView>

MainWindow::MainWindow(): QMainWindow() {    
    //
    // Setup widgets
    //
    resize(1300, 800);
    setWindowTitle("MainWindow");

    const auto central_widget = new QWidget(this);
    setCentralWidget(central_widget);
    
    const auto status_bar = new QStatusBar(this);
    setStatusBar(status_bar);

    const auto menubar = new QMenuBar(this);
    setMenuBar(menubar);
    menubar->setGeometry(QRect(0, 0, 1307, 27));
    const auto menubar_new = menubar->addMenu("New");
    const auto menubar_view = menubar->addMenu("View");

    const auto splitter = new QSplitter(central_widget);
    splitter->setGeometry(QRect(0, 0, 1301, 591));
    splitter->setOrientation(Qt::Horizontal);

    ad_model = new AdModel(this);

    containers_widget = new ContainersWidget(ad_model);
    contents_widget = new ContentsWidget(ad_model);
    attributes_widget = new AttributesWidget();

    splitter->addWidget(containers_widget);
    splitter->addWidget(contents_widget);
    splitter->addWidget(attributes_widget);

    //
    // Setup actions
    //
    const auto action_advanced_view = new QAction("Advanced view", this);
    action_advanced_view->setCheckable(true);
    menubar_view->addAction(action_advanced_view);
    containers_widget->connect_proxy_action(action_advanced_view);
    contents_widget->connect_proxy_action(action_advanced_view);

    const auto action_toggle_dn = new QAction("Show DN");
    action_toggle_dn->setCheckable(true);
    menubar_view->addAction(action_toggle_dn);
    QObject::connect(
        action_toggle_dn, &QAction::triggered,
        containers_widget, &EntryWidget::on_action_toggle_dn);
    QObject::connect(
        action_toggle_dn, &QAction::triggered,
        contents_widget, &EntryWidget::on_action_toggle_dn);

    action_attributes = new QAction("Attributes", this);
    QObject::connect(
        action_attributes, &QAction::triggered,
        this, &MainWindow::on_action_attributes);

    action_delete_entry = new QAction("Delete", this);
    QObject::connect(
        action_delete_entry, &QAction::triggered,
        this, &MainWindow::on_action_delete_entry);

    // Setup "New X" actions
    for (int type_i = NewEntryType::User; type_i < NewEntryType::COUNT; type_i++) {
        NewEntryType type = static_cast<NewEntryType>(type_i);
        QString text = new_entry_type_to_string[type];
        QAction *action = new QAction(text, this);

        QObject::connect(action, &QAction::triggered,
            [type] () {
                create_entry_dialog(type);
            });
        
        new_entry_actions.push_back(action);
        menubar_new->addAction(action);
    }

    // Popup context menu from containers and contents widgets
    QObject::connect(
        containers_widget, &EntryWidget::context_menu_requested,
        this, &MainWindow::popup_entry_context_menu);
    QObject::connect(
        contents_widget, &EntryWidget::context_menu_requested,
        this, &MainWindow::popup_entry_context_menu);

    // Set root index of contents view to selection of containers view
    QObject::connect(
        containers_widget, &ContainersWidget::selected_container_changed,
        contents_widget, &ContentsWidget::on_selected_container_changed);
}

QString MainWindow::get_selected_dn() const {
    QString containers_dn = containers_widget->get_selected_dn();
    QString contents_dn = contents_widget->get_selected_dn();
    
    if (containers_dn != "") {
        return containers_dn;
    } else if (contents_dn != "") {
        return contents_dn;
    } else {
        return "";
    }
}

void MainWindow::on_action_attributes() {
    QString dn = get_selected_dn();
    attributes_widget->set_target_dn(dn);
}

void MainWindow::on_action_delete_entry() {
    QString dn = get_selected_dn();
    delete_entry(dn);
}

void MainWindow::on_action_new_entry(NewEntryType type) {
    QString dn = get_selected_dn();
    create_entry_dialog(type, dn);
}

void MainWindow::popup_entry_context_menu(const QPoint &pos) {
    QMenu menu;

    menu.addAction(action_attributes);
    menu.addAction(action_delete_entry);

    QMenu *submenu_new = menu.addMenu("New");
    for (auto action : new_entry_actions) {
        submenu_new->addAction(action);
    }

    menu.exec(pos, action_attributes);
}
