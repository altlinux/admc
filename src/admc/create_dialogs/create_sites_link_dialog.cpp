#include "create_sites_link_dialog.h"
#include "ui_create_sites_link_dialog.h"
#include "ad_defines.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "ad_filter.h"
#include "globals.h"
#include "ad_config.h"
#include <QListWidget>
#include "managers/icon_manager.h"
#include "ad_utils.h"
#include "utils.h"
#include "status.h"

CreateSitesLinkDialog::CreateSitesLinkDialog(AdInterface &ad, SitesLinkType type_arg, const QString parent_dn_arg, QWidget *parent) :
    CreateObjectDialog(parent),
    ui(new Ui::CreateSitesLinkDialog),
    type(type_arg),
    parent_dn(parent_dn_arg) {

    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    ui->sites_link_common_wget->set_lists_labels(type_arg);

    QIcon item_icon;
    QString search_class;
    if (type_arg == SitesLinkType::Link) {
        ui->clue_label->setText(tr("Site link object must link two or more sites"));
        setWindowTitle(tr("Create site link"));

        item_icon = g_icon_manager->item_icon(ItemIcon_Site);
        search_class = CLASS_SITE;
    } else {
        ui->clue_label->setText(tr("Link bridge object must link two or more site links"));
        setWindowTitle(tr("Create site link bridge"));

        item_icon = g_icon_manager->item_icon(ItemIcon_Site_Link);
        search_class = CLASS_SITE_LINK;
    }

    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, search_class);
    auto search_res = ad.search(g_adconfig->sites_container_dn(), SearchScope_All, filter, {ATTRIBUTE_DN});
    for (const QString &dn : search_res.keys()) {
        QListWidgetItem *item = new QListWidgetItem(item_icon, dn_get_name(dn));
        item->setData(Qt::UserRole, dn);
        ui->sites_link_common_wget->left_list_wget()->addItem(item);
    }
}

CreateSitesLinkDialog::~CreateSitesLinkDialog() {
    delete ui;
}

void CreateSitesLinkDialog::accept() {
    const int min_linked_objects_count = 2;
    if (ui->sites_link_common_wget->right_list_wget()->count() < min_linked_objects_count) {
        const QString warning_text = type == SitesLinkType::Link ?
                    tr("Site link object must link at least two sites") :
                    tr("Link bridge object must link at least two site links");
        message_box_warning(ui->sites_link_common_wget, tr("Error"), warning_text);
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    const QString obj_class = type == SitesLinkType::Link ? CLASS_SITE_LINK : CLASS_SITE_LINK_BRIDGE;

    const QString name = ui->name_edit->text();
    const QString dn = dn_from_name_and_parent(name, parent_dn, CLASS_SUBNET);

    QHash<QString, QList<QString>> attr_map = {
        {ATTRIBUTE_OBJECT_CLASS, {obj_class}}
    };

    const QString link_attr = type == SitesLinkType::Link ? ATTRIBUTE_SITE_LIST : ATTRIBUTE_SITE_LINK_LIST;
    for (int i = 0; i < ui->sites_link_common_wget->right_list_wget()->count(); ++i) {
        auto linked_dn = ui->sites_link_common_wget->right_list_wget()->item(i)->data(Qt::UserRole).toString();
        if (!linked_dn.isEmpty()) {
            attr_map[link_attr] << linked_dn;
        }
    }

    const bool add_success = ad.object_add(dn, attr_map);
    g_status->display_ad_messages(ad, this);

    QString message;
    if (!add_success) {
        message = type == SitesLinkType::Link ? tr("Failed to create site link object %1").arg(name) :
                                                tr("Failed to create site link bridge object %1").arg(name);
        g_status->add_message(message, StatusType_Error);
        return;
    }

    created_dn = dn;
    message = type == SitesLinkType::Link ? tr("Site link object %1 has been successfully created.").arg(name) :
                                            tr("Site link bridge object %1 has been successfully created.").arg(name);
    g_status->add_message(message, StatusType_Success);
    QDialog::accept();
}

QString CreateSitesLinkDialog::get_created_dn() const {
    return created_dn;
}
