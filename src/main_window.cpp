
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

    containers_tree = new ContainersTree(containers_view, ad_model, action_advanced_view);
    contents_list = new ContentsList(contents_view, ad_model, action_advanced_view);
    attributes_list = new AttributesList(attributes_view);

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
        this, &MainWindow::on_action_toggle_dn);

    connect_view_to_entry_context_menu(containers_view);
    connect_view_to_entry_context_menu(contents_view);

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
    actionSomething = new QAction(this);
    actionHere = new QAction(this);
    action_advanced_view = new QAction(this);
    action_advanced_view->setCheckable(true);
    centralwidget = new QWidget(this);
    splitter = new QSplitter(centralwidget);
    splitter->setGeometry(QRect(0, 0, 1301, 591));
    splitter->setOrientation(Qt::Horizontal);
    containers_view = new QTreeView(splitter);
    containers_view->setContextMenuPolicy(Qt::CustomContextMenu);
    containers_view->setAcceptDrops(true);
    containers_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    containers_view->setDragDropMode(QAbstractItemView::DragDrop);
    containers_view->setRootIsDecorated(true);
    containers_view->setItemsExpandable(true);
    containers_view->setExpandsOnDoubleClick(true);
    splitter->addWidget(containers_view);
    containers_view->header()->setVisible(true);
    contents_view = new QTreeView(splitter);
    contents_view->setContextMenuPolicy(Qt::CustomContextMenu);
    contents_view->setAcceptDrops(true);
    contents_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    contents_view->setDragDropMode(QAbstractItemView::DragDrop);
    contents_view->setSelectionMode(QAbstractItemView::SingleSelection);
    contents_view->setRootIsDecorated(false);
    contents_view->setItemsExpandable(false);
    contents_view->setExpandsOnDoubleClick(false);
    splitter->addWidget(contents_view);
    contents_view->header()->setVisible(true);
    attributes_view = new QTreeView(splitter);
    attributes_view->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
    attributes_view->setSelectionMode(QAbstractItemView::NoSelection);
    attributes_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    splitter->addWidget(attributes_view);
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
    menuEdit->addAction(actionSomething);
    menuEdit->addAction(actionHere);
    menuView->addAction(action_advanced_view);

    retranslateUi(this);
}

void MainWindow::retranslateUi(QMainWindow *MainWindow) {
    MainWindow->setWindowTitle(tr("MainWindow"));
    actionSomething->setText(tr("Something"));
    actionHere->setText(tr("Here"));
    action_advanced_view->setText(tr("Advanced view"));
    menubar_new->setTitle(tr("New"));
    menuEdit->setTitle(tr("Edit"));
    menuView->setTitle(tr("View"));
}

void MainWindow::connect_view_to_entry_context_menu(QTreeView *view) {
    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        [this, view] (const QPoint &pos) {
            // Get DN of clicked entry
            QModelIndex index = view->indexAt(pos);

            if (index.isValid()) {
                QPoint global_pos = view->mapToGlobal(pos);
                this->popup_entry_context_menu(global_pos);
            }
        });
}

QString MainWindow::get_selected_dn() {
    QTreeView *focus_view = nullptr;
    
    if (containers_view->hasFocus()) {
        focus_view = containers_view;
    } else if (contents_view->hasFocus()) {
        focus_view = contents_view;
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

void MainWindow::on_action_toggle_dn(bool checked) {
    // TODO: maybe add update_column_visibility() to containers tree as well, and make visibility state an array for all columns 
    containers_view->setColumnHidden(AdModel::Column::DN, !checked);
    contents_list->dn_column_hidden = !checked;
    contents_list->update_column_visibility();
}
