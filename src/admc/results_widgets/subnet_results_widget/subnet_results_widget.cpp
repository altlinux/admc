#include "subnet_results_widget.h"
#include "../ui_results_widget_base.h"
#include "subnet_edit_widget.h"
#include "ad_interface.h"
#include "utils.h"
#include "ad_filter.h"
#include "core/globals.h"
#include "ad_config.h"

SubnetResultsWidget::SubnetResultsWidget(QWidget *parent) :
    ResultsWidgetBase(parent), subnet_edit_wget(new SubnetEditWidget(this)) {

    ui->verticalLayout->addWidget(subnet_edit_wget);
}

void SubnetResultsWidget::update(const AdObject &obj) {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        on_cancel_edit();
        return;
    }

    const QString sites_container_dn = "CN=Sites,CN=Configuration," + g_adconfig->root_domain_dn();
    const QString site_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_SITE);
    const QHash<QString, AdObject> site_objects = ad.search(sites_container_dn, SearchScope_Children,
                                                            site_filter, {ATTRIBUTE_DN, ATTRIBUTE_NAME});
    for (auto dn : site_objects.keys()) {
        site_dn_name_map[dn] = site_objects[dn].get_string(ATTRIBUTE_NAME);
    }

    saved_object = obj;
    subnet_edit_wget->update(obj, site_dn_name_map);
    subnet_edit_wget->set_read_only(true);
}

void SubnetResultsWidget::on_apply() {
    if (changed_attrs().isEmpty()) {
        set_editable(false);
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, this)) {
        on_cancel_edit();
        return;
    }

    const QStringList changed_attr_list = changed_attrs();
    auto current_values_hash = subnet_edit_wget->attr_string_values();
    for (const QString &attr : changed_attr_list) {
        ad.attribute_replace_string(saved_object.get_dn(), attr, current_values_hash[attr], DoStatusMsg_No);
    }

    saved_object = ad.search_object(saved_object.get_dn());
    set_editable(false);
}

void SubnetResultsWidget::on_edit() {
    set_editable(true);
}

void SubnetResultsWidget::on_cancel_edit() {
    subnet_edit_wget->update(saved_object, site_dn_name_map);
    set_editable(false);
}

void SubnetResultsWidget::set_editable(bool is_editable) {
    ResultsWidgetBase::set_editable(is_editable);
    subnet_edit_wget->set_read_only(!is_editable);
}

QStringList SubnetResultsWidget::changed_attrs() {
    QStringList changed_attr_list;
    auto current_values_hash = subnet_edit_wget->attr_string_values();
    for (const QString &attr : current_values_hash.keys()) {
        if (saved_object.get_string(attr) != current_values_hash[attr]) {
            changed_attr_list << attr;
        }
    }

    return changed_attr_list;
}
