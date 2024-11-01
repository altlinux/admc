#include "creation_deletion_permissions_widget.h"

#include "adldap.h"
#include "settings.h"
#include "samba/ndr_security.h"
#include "utils.h"
#include "globals.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QVBoxLayout>

class CreationDeletionRightsSortModel final : public RightsSortModel {
public:
    using RightsSortModel::RightsSortModel;

    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override {
        const QString name_left = source_left.data(Qt::DisplayRole).toString();
        const QString name_right = source_right.data(Qt::DisplayRole).toString();
        const QString object_type_name_left = source_left.data(RightsItemRole_ObjectTypeName).toString();
        const QString object_type_name_right = source_right.data(RightsItemRole_ObjectTypeName).toString();
        SecurityRight left = source_left.data(RightsItemRole_SecurityRight).
                value<SecurityRight>();
        SecurityRight right = source_right.data(RightsItemRole_SecurityRight).
                value<SecurityRight>();
        const bool is_create_left = (left.access_mask == SEC_ADS_CREATE_CHILD);
        const bool is_create_right = (right.access_mask == SEC_ADS_DELETE_CHILD);

        // Rights are sorted by object type name
        if (object_type_name_left != object_type_name_right) {
            return object_type_name_left < object_type_name_right;
        }

        // Create rights are berfore delete rights
        if (is_create_left != is_create_right) {
            return is_create_right;
        }

        return name_left < name_right;
    }
};

CreationDeletionPermissionsWidget::CreationDeletionPermissionsWidget(QWidget *parent) : PermissionsWidget(parent) {
    v_layout->addWidget(rights_view);

    rights_sort_model = new CreationDeletionRightsSortModel(this);
    rights_sort_model->setSourceModel(rights_model);

    rights_view->setModel(rights_sort_model);

    settings_restore_header_state(SETTING_creation_deletion_permissions_header_state, rights_view->header());
}

CreationDeletionPermissionsWidget::~CreationDeletionPermissionsWidget() {
    settings_save_header_state(SETTING_creation_deletion_permissions_header_state, rights_view->header());
}

void CreationDeletionPermissionsWidget::init(const QStringList &target_classes, security_descriptor *sd_arg) {
    PermissionsWidget::init(target_classes, sd_arg);
    append_message_item();

    QStringList inferior_list = g_adconfig->all_inferiors_list(target_classes.last());
    if (!inferior_list.contains(target_classes.last())) {
        inferior_list.append(target_classes.last());
    }

    for (const QString &obj_class : inferior_list) {
        for (const SecurityRight &right : creation_deletion_rights_for_class(g_adconfig, obj_class)) {
            auto row = create_item_row(right);
            rights_model->appendRow(row);
        }
    }
}

QList<QStandardItem *> CreationDeletionPermissionsWidget::create_item_row(const SecurityRight &right) {
    auto row = PermissionsWidget::create_item_row(right);

    const QString obj_class = g_adconfig->guid_to_class(right.object_type);
    row[0]->setData(obj_class, RightsItemRole_ObjectTypeName);

    QString text;
    switch (right.access_mask) {
    case SEC_ADS_CREATE_CHILD:
        text = tr("Create ") + g_adconfig->get_class_display_name(obj_class) + tr(" objects");
        break;
    case SEC_ADS_DELETE_CHILD:
        text = tr("Delete ") + g_adconfig->get_class_display_name(obj_class) + tr(" objects");
        break;
    default:
        text = tr("Undefined");
        row[PermissionColumn_Allowed]->setEnabled(false);
        row[PermissionColumn_Denied]->setEnabled(false);
    }
    row[PermissionColumn_Name]->setText(text);

    return row;
}

bool CreationDeletionPermissionsWidget::right_applies_to_class(const SecurityRight &right, const QString &obj_class) {
    const QString child_obj_class = g_adconfig->guid_to_class(right.object_type);
    const bool appliable = g_adconfig->get_possible_inferiors(obj_class).contains(child_obj_class);
    return appliable;
}

bool CreationDeletionPermissionsWidget::there_are_rights_for_class(const QString &obj_class) {
    return !g_adconfig->get_possible_inferiors(obj_class).isEmpty();
}
