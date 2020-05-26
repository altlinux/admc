
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ad_interface.h"

#include <QMainWindow>
#include <QList>
#include <QTreeView>
#include <QPoint>

class QString;
class AdModel;
class ContainersTree;
class ContentsList;
class AttributesList;
class QAction;
class QWidget;
class QSplitter;
class QTreeView;
class QMenuBar;
class QMenu;
class QStatusBar;

class MainWindow final : public QMainWindow {
Q_OBJECT

public:
    MainWindow();

private:
    void setupUi();
    void retranslateUi(QMainWindow *MainWindow);
    void connect_view_to_entry_context_menu(QTreeView *view);
    
    QString get_selected_dn();
    void on_action_attributes();
    void on_action_delete_entry();
    void on_action_new_entry(NewEntryType type);
    void popup_entry_context_menu(const QPoint &pos);
    void on_action_toggle_dn(bool checked);

    AdModel *ad_model;
    ContainersTree *containers_tree;
    ContentsList *contents_list;
    AttributesList *attributes_list;
    
    QAction *action_toggle_dn;
    QAction *action_attributes;
    QAction *action_delete_entry;
    QAction *action_advanced_view;
    QList<QAction *> new_entry_actions;
    
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
};

#endif /* MAIN_WINDOW_H */
