
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ad_interface.h"

#include <QMainWindow>
#include <QList>
#include <QPoint>

class QString;
class AdModel;
class ContainersTree;
class ContentsList;
class AttributesList;
class QAction;
class QWidget;
class QSplitter;
class QMenuBar;
class QMenu;
class QStatusBar;

class MainWindow final : public QMainWindow {
Q_OBJECT

public:
    MainWindow();

private:
    void setupUi();
    
    QString get_selected_dn() const;
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
    QMenuBar *menubar;
    QMenu *menubar_new;
    QMenu *menuEdit;
    QMenu *menuView;
    QStatusBar *statusbar;
};

#endif /* MAIN_WINDOW_H */
