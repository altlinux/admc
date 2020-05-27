
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ad_interface.h"

#include <QMainWindow>

class QString;
class AdModel;
class ContainersWidget;
class ContentsWidget;
class AttributesWidget;

class MainWindow final : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow();

private slots:
    void on_action_attributes();
    void on_action_delete_entry();
    void on_action_new_user();
    void on_action_new_computer();
    void on_action_new_group();
    void on_action_new_ou();

private:
    QString get_selected_dn() const;
    void on_action_new_entry_generic(NewEntryType type);

    AdModel *ad_model;
    ContainersWidget *containers_widget;
    ContentsWidget *contents_widget;
    AttributesWidget *attributes_widget;
};

#endif /* MAIN_WINDOW_H */
