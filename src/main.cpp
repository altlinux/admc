
#include "containers_tree.h"
#include "contents_list.h"
#include "attributes_list.h"
#include "ad_filter.h"
#include "ad_model.h"
#include "attributes_model.h"
#include "create_entry_dialog.h"
#include "ad_interface.h"

#include <QApplication>
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

ContentsList *contents_list;
AttributesList *attributes_list;
QAction *action_attributes;
QAction *action_delete_entry;
QList<QAction *> new_entry_actions;

QAction *actionSomething;
QAction *actionHere;
QAction *action_advanced_view;
QWidget *centralwidget;
QSplitter *splitter;
QTreeView *containers_view;
QTreeView *contents_view;
QTreeView *attributes_view;
QMenuBar *menubar;
QMenu *menubar_new;
QMenu *menuEdit;
QMenu *menuView;
QStatusBar *statusbar;

QString get_selected_dn() {
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

void on_action_toggle_dn(bool checked) {
    // TODO: maybe add update_column_visibility() to containers tree as well, and make visibility state an array for all columns 
    containers_view->setColumnHidden(AdModel::Column::DN, !checked);
    contents_list->dn_column_hidden = !checked;
    contents_list->update_column_visibility();
}

void retranslateUi(QMainWindow *MainWindow) {
    MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
    actionSomething->setText(QApplication::translate("MainWindow", "Something", nullptr));
    actionHere->setText(QApplication::translate("MainWindow", "Here", nullptr));
    action_advanced_view->setText(QApplication::translate("MainWindow", "Advanced view", nullptr));
    menubar_new->setTitle(QApplication::translate("MainWindow", "New", nullptr));
    menuEdit->setTitle(QApplication::translate("MainWindow", "Edit", nullptr));
    menuView->setTitle(QApplication::translate("MainWindow", "View", nullptr));
}

void setupUi(QMainWindow *MainWindow) {
    if (MainWindow->objectName().isEmpty())
        MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
    MainWindow->resize(1307, 795);
    actionSomething = new QAction(MainWindow);
    actionSomething->setObjectName(QString::fromUtf8("actionSomething"));
    actionHere = new QAction(MainWindow);
    actionHere->setObjectName(QString::fromUtf8("actionHere"));
    action_advanced_view = new QAction(MainWindow);
    action_advanced_view->setObjectName(QString::fromUtf8("action_advanced_view"));
    action_advanced_view->setCheckable(true);
    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    splitter = new QSplitter(centralwidget);
    splitter->setObjectName(QString::fromUtf8("splitter"));
    splitter->setGeometry(QRect(0, 0, 1301, 591));
    splitter->setOrientation(Qt::Horizontal);
    containers_view = new QTreeView(splitter);
    containers_view->setObjectName(QString::fromUtf8("containers_view"));
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
    contents_view->setObjectName(QString::fromUtf8("contents_view"));
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
    attributes_view->setObjectName(QString::fromUtf8("attributes_view"));
    attributes_view->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
    attributes_view->setSelectionMode(QAbstractItemView::NoSelection);
    attributes_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    splitter->addWidget(attributes_view);
    MainWindow->setCentralWidget(centralwidget);
    menubar = new QMenuBar(MainWindow);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 1307, 27));
    menubar_new = new QMenu(menubar);
    menubar_new->setObjectName(QString::fromUtf8("menubar_new"));
    menuEdit = new QMenu(menubar);
    menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
    menuView = new QMenu(menubar);
    menuView->setObjectName(QString::fromUtf8("menuView"));
    MainWindow->setMenuBar(menubar);
    statusbar = new QStatusBar(MainWindow);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    MainWindow->setStatusBar(statusbar);

    menubar->addAction(menubar_new->menuAction());
    menubar->addAction(menuEdit->menuAction());
    menubar->addAction(menuView->menuAction());
    menuEdit->addAction(actionSomething);
    menuEdit->addAction(actionHere);
    menuView->addAction(action_advanced_view);

    retranslateUi(MainWindow);
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
    setupUi(&main_window);

    AdModel ad_model;

    ContainersTree containers_tree(containers_view, &ad_model, action_advanced_view);
    ContentsList contents_list(contents_view, &ad_model, action_advanced_view);
    attributes_list = new AttributesList(attributes_view);

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

    // DN toggle
    QAction action_toggle_dn("Show DN");
    action_toggle_dn.setCheckable(true);
    menuView->addAction(&action_toggle_dn);
    QObject::connect(
        &action_toggle_dn, &QAction::triggered,
        on_action_toggle_dn);

    connect_view_to_entry_context_menu(*containers_view);
    connect_view_to_entry_context_menu(*contents_view);

    // Set root index of contents view to selection of containers view
    QObject::connect(
        &containers_tree, &ContainersTree::selected_container_changed,
        &contents_list, &ContentsList::on_selected_container_changed);
    
    // Add menubar actions
    for (auto a : new_entry_actions) {
        menubar_new->addAction(a);
    }

    main_window.show();

    return app.exec();
}
