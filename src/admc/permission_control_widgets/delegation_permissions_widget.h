#ifndef DELEGATION_PERMISSIONS_WIDGET_H
#define DELEGATION_PERMISSIONS_WIDGET_H

#include "permissions_widget.h"

struct SecurityRight;
class QRadioButton;
class AdConfig;

class DelegationPermissionsWidget final : public PermissionsWidget {

    Q_OBJECT

    enum TaskDelegationColumn {
        TaskDelegationColumn_Name = PermissionColumn_Name,
        TaskDelegationColumn_Assigned = PermissionColumn_Allowed,
        TaskDelegationColumn_COUNT
    };

public:
    DelegationPermissionsWidget(QWidget *parent = nullptr);
    ~DelegationPermissionsWidget();

    virtual void init(const QStringList &target_classes,
                      security_descriptor *sd_arg) override;
    virtual void update_permissions(AppliedObjects applied_objs, const QString &appliable_child_class) override;
    virtual void update_permissions() override;

private:
    void append_common_tasks();

    QList<QStandardItem*> create_item_row(const QString &name, const QList<SecurityRight> rights, const QString &object_type_name);

    virtual bool there_are_rights_for_class(const QString &obj_class) override;
    virtual bool right_applies_to_class(const SecurityRight &right, const QString &obj_class) override;

    void update_task_check_state(QStandardItem *item);

    virtual void make_model_rights_read_only() override;

    QString target_class;
};

#endif // DELEGATION_PERMISSIONS_WIDGET_H
