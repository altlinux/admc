#include "read_write_permissions_widget.h"

#include "adldap.h"
#include "settings.h"
#include "samba/ndr_security.h"
#include "utils.h"
#include "globals.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QVBoxLayout>

class ReadWriteRightsSortModel final : public RightsSortModel {
public:
    using RightsSortModel::RightsSortModel;

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override {
        const QString name_left = source_left.data(Qt::DisplayRole).toString();
        const QString name_right = source_right.data(Qt::DisplayRole).toString();
        const QString object_type_name_left = source_left.data(RightsItemRole_ObjectTypeName).toString();
        const QString object_type_name_right = source_right.data(RightsItemRole_ObjectTypeName).toString();

        SecurityRight left = source_left.data(RightsItemRole_SecurityRight).
                value<SecurityRight>();
        SecurityRight right = source_right.data(RightsItemRole_SecurityRight).
                value<SecurityRight>();

        // Rights are sorted by object type name
        if (object_type_name_left != object_type_name_right) {
            return object_type_name_left < object_type_name_right;
        }

        const bool is_read_left = (left.access_mask == SEC_ADS_READ_PROP);
        const bool is_read_right = (right.access_mask == SEC_ADS_READ_PROP);
        // Read rights are before write rights
        if (is_read_left != is_read_right) {
            return is_read_left;
        }

        return name_left < name_right;
    }
};

ReadWritePermissionsWidget::ReadWritePermissionsWidget(QWidget *parent) : PermissionsWidget(parent) {
    v_layout->addWidget(rights_view);

    rights_sort_model = new ReadWriteRightsSortModel(this);
    rights_sort_model->setSourceModel(rights_model);

    rights_view->setModel(rights_sort_model);

    settings_restore_header_state(SETTING_read_write_permissions_header_state, rights_view->header());
}

ReadWritePermissionsWidget::~ReadWritePermissionsWidget() {
    settings_save_header_state(SETTING_read_write_permissions_header_state, rights_view->header());
}

void ReadWritePermissionsWidget::init(const QStringList &target_classes, security_descriptor *sd_arg) {
    PermissionsWidget::init(target_classes, sd_arg);
    append_message_item();

    QStringList obj_class_list = g_adconfig->all_inferiors_list(target_classes.last());
    if (!obj_class_list.contains(target_classes.last())) {
        obj_class_list.append(target_classes.last());
    }

    // Use set to avoid duplicates
    QSet<QString> all_attrs;
    for (const QString &obj_class : obj_class_list) {
        const QStringList obj_class_attrs = g_adconfig->get_permissionable_attributes(obj_class);
        all_attrs.unite(QSet<QString>(obj_class_attrs.begin(), obj_class_attrs.end()));
    }

    for (const QString &attribute : all_attrs) {
        for (const SecurityRight &right : read_write_property_rights(g_adconfig, attribute)) {
            auto row = create_item_row(right);
            rights_model->appendRow(row);
        }
    }
}

QList<QStandardItem *> ReadWritePermissionsWidget::create_item_row(const SecurityRight &right) {
    auto row = PermissionsWidget::create_item_row(right);

    const QString attr = g_adconfig->guid_to_attribute(right.object_type);
    row[0]->setData(attr, RightsItemRole_ObjectTypeName);

    QString text;
    switch (right.access_mask) {
    case SEC_ADS_WRITE_PROP:
        text = tr("Write ") + g_adconfig->get_column_display_name(attr) + tr(" property");
        break;
    case SEC_ADS_READ_PROP:
        text = tr("Read ") + g_adconfig->get_column_display_name(attr) + tr(" property");
        break;
    default:
        text = tr("Undefined");
        row[PermissionColumn_Allowed]->setEnabled(false);
        row[PermissionColumn_Denied]->setEnabled(false);
    }
    row[PermissionColumn_Name]->setText(text);

    return row;
}

bool ReadWritePermissionsWidget::right_applies_to_class(const SecurityRight &right, const QString &obj_class) {
    const QString attr = g_adconfig->guid_to_attribute(right.object_type);
    return g_adconfig->get_permissionable_attributes(obj_class).contains(attr);
}

bool ReadWritePermissionsWidget::there_are_rights_for_class(const QString &obj_class) {
    return !g_adconfig->get_permissionable_attributes(obj_class).isEmpty();
}
