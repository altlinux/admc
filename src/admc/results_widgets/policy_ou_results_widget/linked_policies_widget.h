#ifndef LINKED_POLICIES_WIDGET_H
#define LINKED_POLICIES_WIDGET_H

#include "gplink.h"

#include <QWidget>

class QStandardItemModel;
class QStandardItem;
class QMenu;
class ResultsView;
class ConsoleWidget;
class ADMCTestPolicyOUResultsWidget;
class AdObject;
class AdInterface;
class DragDropLinksModel;
class QAction;

namespace Ui {
class LinkedPoliciesWidget;
}

class LinkedPoliciesWidget final : public QWidget {
    Q_OBJECT

public:
    explicit LinkedPoliciesWidget(ConsoleWidget *console_arg, QWidget *parent = nullptr);
    ~LinkedPoliciesWidget();

    // Loads links for given OU. Nothing is done if given
    // index is not an OU in policy tree.
    void update(const QModelIndex &ou_index);

signals:
    void gplink_changed(const QModelIndex &index);

private:
    Ui::LinkedPoliciesWidget *ui;

    ConsoleWidget *console;
    DragDropLinksModel *model;
    Gplink gplink;
    QString ou_dn;
    QMenu *context_menu;
    QAction *set_all_checked_action;
    QAction *set_all_unchecked_action;
    QAction *edit_policy_action;

    void on_item_changed(QStandardItem *item);
    void open_context_menu(const QPoint &pos);
    void remove_link();
    void move_up();
    void move_down();
    void update_link_items();
    void modify_gplink(void (*modify_function)(Gplink &, const QString &));
    void update_policy_link_icons(const QModelIndex &changed_item_index, bool is_checked, GplinkOption option);
    QList<AdObject> gpo_object_list(AdInterface &ad);
    void load_item_row(const AdObject &gpo_object, QList<QStandardItem*> row);
    void set_all_column_check_state(int column, bool checked);
    void on_set_checked_all();
    void on_set_unchecked_all();
    void on_edit_policy();
};

#endif // LINKED_POLICIES_WIDGET_H
