#ifndef LINKED_POLICIES_WIDGET_H
#define LINKED_POLICIES_WIDGET_H

#include "gplink.h"

#include <QWidget>
#include <QSet>

class QStandardItemModel;
class QStandardItem;
class QMenu;
class ResultsView;
class ConsoleWidget;
class ADMCTestPolicyOUResultsWidget;
class AdObject;
class AdInterface;

namespace Ui {
class LinkedPoliciesWidget;
}

class LinkedPoliciesWidget final : public QWidget {
    Q_OBJECT

    enum LinkedPoliciesColumn {
        LinkedPoliciesColumn_Order,
        LinkedPoliciesColumn_Name,
        LinkedPoliciesColumn_Enforced,
        LinkedPoliciesColumn_Disabled,

        LinkedPoliciesColumn_COUNT,
    };

    enum LinkedPoliciesRole {
        LinkedPoliciesRole_DN = Qt::UserRole + 1,

        LinkedPoliciesRole_COUNT,
    };

    const QSet<LinkedPoliciesColumn> option_columns = {
        LinkedPoliciesColumn_Enforced,
        LinkedPoliciesColumn_Disabled,
    };

    const QHash<LinkedPoliciesColumn, GplinkOption> column_to_option = {
        {LinkedPoliciesColumn_Enforced, GplinkOption_Enforced},
        {LinkedPoliciesColumn_Disabled, GplinkOption_Disabled},
    };

public:
    explicit LinkedPoliciesWidget(QWidget *parent = nullptr);
    ~LinkedPoliciesWidget();

    // Loads links for given OU. Nothing is done if given
    // index is not an OU in policy tree.
    void update(const QModelIndex &ou_index);

    void set_console(ConsoleWidget *console_arg);

signals:
    void gplink_changed(const QModelIndex &index);

private:
    Ui::LinkedPoliciesWidget *ui;

    ConsoleWidget *console;
    QStandardItemModel *model;
    Gplink gplink;
    QString ou_dn;
    QMenu *context_menu;

    void on_item_changed(QStandardItem *item);
    void open_context_menu(const QPoint &pos);
    void remove_link();
    void move_up();
    void move_down();
    void reload_gplink();
    void modify_gplink(void (*modify_function)(Gplink &, const QString &));
    void change_policy_icon(const QString &policy_dn, bool is_checked, GplinkOption option);
    QList<AdObject> gpo_object_list(AdInterface &ad);
    void update_item_row(const AdObject &gpo_object, QList<QStandardItem*> row);
};

#endif // LINKED_POLICIES_WIDGET_H
