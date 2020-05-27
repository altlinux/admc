
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ad_interface.h"

#include <QMainWindow>
#include <QList>
#include <QPoint>

class QString;
class AdModel;
class ContainersWidget;
class ContentsWidget;
class AttributesWidget;
class QAction;
class QWidget;
class QSplitter;
class QMenuBar;
class QMenu;
class QStatusBar;

class MainWindow final : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow();

private:
    QString get_selected_dn() const;
    void on_action_attributes();
    void on_action_delete_entry();
    void on_action_new_entry(NewEntryType type);
    void popup_entry_context_menu(const QPoint &pos);

    AdModel *ad_model;
    ContainersWidget *containers_widget;
    ContentsWidget *contents_widget;
    AttributesWidget *attributes_widget;
    
    QAction *action_attributes;
    QAction *action_delete_entry;
    QList<QAction *> new_entry_actions;
};

#endif /* MAIN_WINDOW_H */
