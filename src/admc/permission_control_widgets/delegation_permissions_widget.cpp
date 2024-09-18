#include "delegation_permissions_widget.h"

#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"
#include "common_task_manager.h"

#include "samba/ndr_security.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHeaderView>

class DelegationSortModel final : public RightsSortModel {
public:
    using RightsSortModel::RightsSortModel;

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override {
        const QString name_left = source_left.data(Qt::DisplayRole).toString();
        const QString name_right = source_right.data(Qt::DisplayRole).toString();
        const QString object_type_name_left = source_left.data(RightsItemRole_ObjectTypeName).toString();
        const QString object_type_name_right = source_right.data(RightsItemRole_ObjectTypeName).toString();

        // Rights are sorted by object type name
        if (object_type_name_left != object_type_name_right) {
            return object_type_name_left < object_type_name_right;
        }

        return name_left < name_right;
    }
};

DelegationPermissionsWidget::DelegationPermissionsWidget(QWidget *parent) : PermissionsWidget(parent) {
    v_layout->addWidget(rights_view);

    set_horizontal_header_labels_from_map(rights_model,
        {
            {TaskDelegationColumn_Name, tr("Name")},
            {TaskDelegationColumn_Assigned, tr("Assigned")}
        });;

    rights_sort_model = new DelegationSortModel(this);
    rights_sort_model->setSourceModel(rights_model);

    rights_view->setModel(rights_sort_model);
    rights_view->setColumnHidden(TaskDelegationColumn_COUNT, true);

    settings_restore_header_state(SETTING_delegation_permissions_header_state, rights_view->header());

    common_task_manager->init(g_adconfig);
}

DelegationPermissionsWidget::~DelegationPermissionsWidget() {
    settings_save_header_state(SETTING_delegation_permissions_header_state, rights_view->header());
}

void DelegationPermissionsWidget::init(const QStringList &target_classes, security_descriptor *sd_arg) {
    PermissionsWidget::init(target_classes, sd_arg);
    append_message_item();

    target_class = target_classes.last();

    append_common_tasks();

    show_no_rights_message(!there_are_rights_for_class(target_classes.last()));

    for (int column = 0; column < (int)TaskDelegationColumn_COUNT; ++column) {
        rights_view->resizeColumnToContents(column);
    }
}

void DelegationPermissionsWidget::append_common_tasks() {
    // Add delegation permissions for corresponding child object classes
    QStringList child_classes = g_adconfig->get_possible_inferiors(target_class);
    QList<CommonTask> child_class_tasks;
    for (const QString &obj_class : common_task_manager->tasks_object_classes()) {
        if (!child_classes.contains(obj_class)) {
            continue;
        }
        child_class_tasks.append(common_task_manager->class_common_task_rights_map[obj_class]);
    }

    for (CommonTask task : child_class_tasks) {
        QString obj_type_name;
        for (const QString &obj_class : common_task_manager->class_common_task_rights_map.keys()) {
            if (common_task_manager->class_common_task_rights_map[obj_class].contains(task)) {
                obj_type_name = obj_class;
            }
        }
        auto row = create_item_row(common_task_manager->common_task_name[task], common_task_manager->common_task_rights[task],
                                   obj_type_name);
        rights_model->appendRow(row);
    }
}

bool DelegationPermissionsWidget::there_are_rights_for_class(const QString &obj_class) {
    const QStringList task_class_list = common_task_manager->class_common_task_rights_map.keys();
    QSet<QString> task_classes_set = QSet<QString>(task_class_list.begin(), task_class_list.end());

    const QStringList obj_class_list = g_adconfig->all_inferiors_list(obj_class);
    QSet<QString> obj_class_set = QSet<QString>(obj_class_list.begin(), obj_class_list.end());
    obj_class_set.insert(obj_class);

    return task_classes_set.intersects(obj_class_set);
}

bool DelegationPermissionsWidget::right_applies_to_class(const SecurityRight &right, const QString &obj_class) {
    Q_UNUSED(right)
    Q_UNUSED(obj_class)
    return true;
}

void DelegationPermissionsWidget::update_task_check_state(QStandardItem *item) {
    const QList<SecurityRight> rights = item->data(RightsItemRole_SecurityRightList).value<QList<SecurityRight>>();
    // For tasks with multiple ACEs right state have to match.
    QList<Qt::CheckState> right_state_checked_list;

    const QModelIndex sibling_index = rights_model->index(item->row(), TaskDelegationColumn_Assigned);
    QStandardItem *checkable_sibling_item = rights_model->itemFromIndex(sibling_index);
    checkable_sibling_item->setEnabled(true);

    int inherited_rights_count = 0;

    for (const SecurityRight &right : rights) {
        const SecurityRightState state = security_descriptor_get_right_state(sd, trustee, right);

        if (right_state_checked_list.contains(Qt::Unchecked)) {
            continue;
        }

        const bool object_ace_state = state.get(SecurityRightStateInherited_No, SecurityRightStateType_Allow);
        const bool inherited_ace_state = state.get(SecurityRightStateInherited_Yes, SecurityRightStateType_Allow);

        Qt::CheckState check_state = object_ace_state || inherited_ace_state ? Qt::Checked : Qt::Unchecked;
        checkable_sibling_item->setCheckState(check_state);

        right_state_checked_list.append(check_state);
        if (inherited_ace_state) {
            ++inherited_rights_count;
        }
    }

    // Disable task if all its ACEs are inherited
    if (inherited_rights_count == rights.size()) {
        checkable_sibling_item->setEnabled(false);
    }
}

void DelegationPermissionsWidget::make_model_rights_read_only() {
    // NOTE: important to ignore this signal because
    // it's slot reloads the rights model
    ignore_item_changed_signal = true;

    for (int row = 0; row < rights_model->rowCount(); row++) {
        QStandardItem *item = rights_model->item(row, TaskDelegationColumn_Assigned);
        item->setEnabled(false);
    }

    ignore_item_changed_signal = false;
}

void DelegationPermissionsWidget::update_permissions(AppliedObjects applied_objs, const QString &appliable_child_class) {
    Q_UNUSED(applied_objs)
    Q_UNUSED(appliable_child_class)
    return;
}

void DelegationPermissionsWidget::update_permissions() {
    // NOTE: this flag is turned on so that
    // on_item_changed() slot doesn't react to us
    // changing state of items
    ignore_item_changed_signal = true;

    for (int row = 0; row < rights_model->rowCount(); row++) {
        const QModelIndex index = rights_model->index(row, 0);
        if (!index.isValid() || item_is_message(index)) {
            continue;
        }

        QStandardItem *main_item = rights_model->itemFromIndex(index);
        update_task_check_state(main_item);
    }

    if (read_only) {
        make_model_rights_read_only();
    }

    ignore_item_changed_signal = false;
}

QList<QStandardItem *> DelegationPermissionsWidget::create_item_row(const QString &name, const QList<SecurityRight> rights, const QString &object_type_name) {
    const QList<QStandardItem *> row = make_item_row(TaskDelegationColumn_COUNT);

    row[TaskDelegationColumn_Name]->setText(name);
    row[TaskDelegationColumn_Name]->setEditable(false);
    row[TaskDelegationColumn_Assigned]->setCheckable(true);
    row[TaskDelegationColumn_Assigned]->setEditable(false);

    row[0]->setData(object_type_name, RightsItemRole_ObjectTypeName);

    QVariant right_data;
    right_data.setValue(rights);
    row[0]->setData(right_data, RightsItemRole_SecurityRightList);
    return row;
}
