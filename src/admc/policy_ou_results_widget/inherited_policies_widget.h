#ifndef INHERITED_POLICIES_WIDGET_H
#define INHERITED_POLICIES_WIDGET_H

#include <QWidget>
#include <QModelIndex>

namespace Ui {
class InheritedPoliciesWidget;
}

class QStandardItemModel;
class ConsoleWidget;
class QStandardItem;
class QStringList;

class InheritedPoliciesWidget final : public QWidget
{
    Q_OBJECT

public:

    enum InheritedPoliciesColumns {
        InheritedPoliciesColumns_Prority,
        InheritedPoliciesColumns_Name,
        InheritedPoliciesColumns_Location,
        InheritedPoliciesColumns_Status,

        InheritedPoliciesColumns_COUNT
    };

    enum RowRoles {
       RowRole_DN = Qt::UserRole + 1,
       RowRole_IsEnforced,
    };

    explicit InheritedPoliciesWidget(QWidget *parent = nullptr);
    ~InheritedPoliciesWidget();

    void update(const QModelIndex &index, ConsoleWidget *console_widget);
    void hide_not_enforced_inherited_links(bool hide);

private:
    QStandardItemModel *model;
    ConsoleWidget *console;
    Ui::InheritedPoliciesWidget *ui;
    QModelIndex selected_scope_index;

    void add_enabled_policy_items(const QModelIndex &index, bool inheritance_blocked = false);
    void set_priority_to_items();
    void load_item(const QList<QStandardItem *> row, const QModelIndex &ou_index, const QString &policy_dn, bool is_enforced);
};

#endif // INHERITED_POLICIES_WIDGET_H
