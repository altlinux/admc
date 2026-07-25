#include "subnet_edit_widget.h"
#include "ui_subnet_edit_widget.h"
#include "ad_object.h"
#include "ad_defines.h"
#include "ad_utils.h"
#include "globals.h"
#include "core/managers/icon_manager.h"


SubnetEditWidget::SubnetEditWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubnetEditWidget) {
    ui->setupUi(this);

    ui->prefix_edit->setReadOnly(true);
}

SubnetEditWidget::~SubnetEditWidget() {
    delete ui;
}

void SubnetEditWidget::update(const AdObject &subnet_obj, QHash<QString, QString> sites_dn_name_map) {
    ui->sites_cmb_box->clear();
    for (auto dn : sites_dn_name_map.keys()) {
        ui->sites_cmb_box->addItem(g_icon_manager->category_icon(OBJECT_CATEGORY_SITE),
                                   sites_dn_name_map[dn], dn);
    }

    const QString site_dn = subnet_obj.get_string(ATTRIBUTE_SITE_OBJECT);
    int cmbbox_idx = ui->sites_cmb_box->findData(site_dn);
    if (cmbbox_idx >= 0) {
        ui->sites_cmb_box->setCurrentIndex(cmbbox_idx);
    }
    else {
        ui->sites_cmb_box->insertItem(0, g_icon_manager->category_icon(OBJECT_CATEGORY_SITE),
                                      dn_get_name(site_dn), site_dn);
    }

    const QString description = subnet_obj.get_string(ATTRIBUTE_DESCRIPTION);
    ui->description_edit->setText(description);

    const QString prefix = subnet_obj.get_string(ATTRIBUTE_NAME);
    ui->prefix_edit->setText(prefix);

    const QString location = subnet_obj.get_string(ATTRIBUTE_LOCATION);
    ui->location_edit->setText(location);
}

void SubnetEditWidget::set_read_only(bool read_only) {
    ui->description_edit->setReadOnly(read_only);
    ui->sites_cmb_box->setDisabled(read_only);
    ui->location_edit->setReadOnly(read_only);
}

QComboBox *SubnetEditWidget::subnet_sites_cmbbox() {
    return ui->sites_cmb_box;
}

QHash<QString, QString> SubnetEditWidget::attr_string_values() {
    QHash<QString, QString> attr_value_map;

    const QString site_obj_dn = ui->sites_cmb_box->currentData().toString();
    attr_value_map[ATTRIBUTE_SITE_OBJECT] = site_obj_dn;

    attr_value_map[ATTRIBUTE_DESCRIPTION] = ui->description_edit->text();
    attr_value_map[ATTRIBUTE_LOCATION] = ui->location_edit->text();

    return attr_value_map;
}
