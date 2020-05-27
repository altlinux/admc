
#include "main_window.h"
#include "containers_tree.h"
#include "contents_list.h"
#include "attributes_list.h"
#include "ad_filter.h"
#include "ad_model.h"
#include "attributes_model.h"
#include "create_entry_dialog.h"

#include <QString>
#include <QAction>
#include <QVariant>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTreeView>
#include <QHeaderView>

MainWindow::MainWindow(): QMainWindow() {
    setupUi();

    ad_model = new AdModel();

    containers_tree = new ContainersTree(ad_model, action_advanced_view);
    contents_list = new ContentsList(ad_model, action_advanced_view);
    attributes_list = new AttributesList();

    splitter->addWidget(containers_tree);
    splitter->addWidget(contents_list);
    splitter->addWidget(attributes_list);

    // Setup actions
    action_attributes = new QAction("Attributes");
    action_delete_entry = new QAction("Delete");
    QObject::connect(
        action_attributes, &QAction::triggered,
        this, &MainWindow::on_action_attributes);
    QObject::connect(
        action_delete_entry, &QAction::triggered,
        this, &MainWindow::on_action_delete_entry);

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

    // DN toggle
    action_toggle_dn = new QAction("Show DN");
    action_toggle_dn->setCheckable(true);
    menuView->addAction(action_toggle_dn);
    QObject::connect(
        action_toggle_dn, &QAction::triggered,
        containers_tree, &EntryWidget::on_action_toggle_dn);
    QObject::connect(
        action_toggle_dn, &QAction::triggered,
        contents_list, &EntryWidget::on_action_toggle_dn);


    QObject::connect(
        containers_tree, &EntryWidget::context_menu_requested,
        this, &MainWindow::popup_entry_context_menu);
    QObject::connect(
        contents_list, &EntryWidget::context_menu_requested,
        this, &MainWindow::popup_entry_context_menu);

    // Set root index of contents view to selection of containers view
    QObject::connect(
        containers_tree, &ContainersTree::selected_container_changed,
        contents_list, &ContentsList::on_selected_container_changed);
    
    // Add menubar actions
    for (auto a : new_entry_actions) {
        menubar_new->addAction(a);
    }
}

void MainWindow::setupUi() {
    this->resize(1307, 795);
    action_advanced_view = new QAction(this);
    action_advanced_view->setCheckable(true);
    centralwidget = new QWidget(this);
    splitter = new QSplitter(centralwidget);
    splitter->setGeometry(QRect(0, 0, 1301, 591));
    splitter->setOrientation(Qt::Horizontal);
    
    this->setCentralWidget(centralwidget);
    menubar = new QMenuBar(this);
    menubar->setGeometry(QRect(0, 0, 1307, 27));
    menubar_new = new QMenu(menubar);
    menuEdit = new QMenu(menubar);
    menuView = new QMenu(menubar);
    this->setMenuBar(menubar);
    statusbar = new QStatusBar(this);
    this->setStatusBar(statusbar);

    menubar->addAction(menubar_new->menuAction());
    menubar->addAction(menuEdit->menuAction());
    menubar->addAction(menuView->menuAction());
    menuView->addAction(action_advanced_view);

    setWindowTitle(tr("MainWindow"));
    action_advanced_view->setText(tr("Advanced view"));
    menubar_new->setTitle(tr("New"));
    menuEdit->setTitle(tr("Edit"));
    menuView->setTitle(tr("View"));
}

QString MainWindow::get_selected_dn() const {
    QString containers_dn = containers_tree->get_selected_dn();
    QString contents_dn = contents_list->get_selected_dn();
    
    if (containers_dn != "") {
        return containers_dn;
    } else if (contents_dn != "") {
        return contents_dn;
    } else {
        return "";
    }
}

void MainWindow::on_action_attributes() {
    auto dn = get_selected_dn();
    attributes_list->set_target_dn(dn);
}

void MainWindow::on_action_delete_entry() {
    auto dn = get_selected_dn();
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
