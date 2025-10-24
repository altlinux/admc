#include "fsmo_table_widget.h"
#include "ui_fsmo_table_widget.h"

#include "fsmo/fsmo_utils.h"
#include <QMessageBox>
#include "managers/icon_manager.h"
#include "status.h"
#include "globals.h"
#include "utils.h"
#include "adldap.h"
#include <QPushButton>


FsmoTableWidget::FsmoTableWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FsmoTableWidget) {
    ui->setupUi(this);

    ui->fsmo_group_box->setAlignment(Qt::AlignLeft);
    ui->fsmo_table->setColumnCount(FsmoColumn_COUNT);
    ui->fsmo_table->setRowCount(int(FSMORole_COUNT));
    ui->fsmo_table->setHorizontalHeaderLabels({tr("FSMO role"), tr("Host"), tr("Role capture")});
    ui->fsmo_table->setSortingEnabled(false);
    for (int role = 0; role < FSMORole_COUNT; ++role) {
        QTableWidgetItem *fsmo_item = new QTableWidgetItem(g_icon_manager->category_icon(ADMC_CATEGORY_FSMO_ROLE),
                                                           string_fsmo_role(FSMORole(role)));
        ui->fsmo_table->setItem(role, (int)FsmoColumn_Role, fsmo_item);
        fsmo_item->setFlags(fsmo_item->flags() & ~Qt::ItemIsEditable);
        fsmo_item->setData(ItemRole_FsmoRole, role);
    }

    ui->fsmo_table->resizeColumnToContents(0);
    ui->fsmo_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    for (int i = 1; i < ui->fsmo_table->columnCount(); ++i) {
        ui->fsmo_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    }

    ui->fsmo_table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->fsmo_table->verticalHeader()->setHidden(true);


    connect(ui->fsmo_table, &QTableWidget::itemChanged, this, &FsmoTableWidget::on_fsmo_capture);
}

FsmoTableWidget::~FsmoTableWidget() {
    delete ui;
}

void FsmoTableWidget::update(AdInterface &ad, const QList<AdObject> &hosts_list) {
    for (const AdObject &host_obj : hosts_list) {
        dn_dns_name_map[host_obj.get_dn()] = host_obj.get_string(ATTRIBUTE_DNS_HOST_NAME);
    }

    current_dc_dns_name = current_dc_dns_host_name(ad);
    for (int row = 0; row < FSMORole_COUNT; ++row) {
        const QString current_master = current_master_for_role(ad, FSMORole(row));
        QTableWidgetItem *host_item = new QTableWidgetItem(g_icon_manager->item_icon(ItemIcon_Domain),
                                                           current_master);
        ui->fsmo_table->setItem(row, (int)FsmoColumn_Host, host_item);
        host_item->setFlags(host_item->flags() & ~Qt::ItemIsEditable);

        QPushButton *capture_button = new QPushButton(tr("Capture"));
        ui->fsmo_table->setCellWidget(row, FsmoColumn_Capture, capture_button);
        capture_button->setProperty(button_property_row, row);
        connect(capture_button, &QPushButton::clicked, this, &FsmoTableWidget::on_fsmo_capture);

        if (current_dc_dns_name == current_master) {
            capture_button->setText(tr("Captured"));
            capture_button->setDisabled(true);
        }
    }
}

void FsmoTableWidget::update_role_owner(const QString &new_master_dn, const QString &fsmo_role_dn) {
    const QString new_master_dns_name = dn_dns_name_map.value(new_master_dn, QString());
    if (new_master_dns_name.isEmpty()) {
        return;
    }

    FSMORole role;
    if (fsmo_role_from_dn(fsmo_role_dn, role)) {
        for (int row = 0; row < ui->fsmo_table->rowCount(); ++row) {
            QTableWidgetItem* role_item = ui->fsmo_table->item(row, FsmoColumn_Role);
            if (role_item && role_item->data(Qt::UserRole).toInt() == (int)role) {
                ui->fsmo_table->item(row, FsmoColumn_Host)->setText(new_master_dns_name);
                if (new_master_dns_name == current_dc_dns_name) {
                    QPushButton *btn = qobject_cast<QPushButton*>(ui->fsmo_table->cellWidget(row, FsmoColumn_Capture));
                    if (btn) {
                        btn->setText(tr("Captured"));
                        btn->setDisabled(true);
                    }
                }
            }
        }
    }
}

void FsmoTableWidget::on_fsmo_capture() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    int row = btn->property(button_property_row).toInt();
    const QString role_str = ui->fsmo_table->item(row, FsmoColumn_Role)->text();
    QMessageBox::StandardButton ans = QMessageBox::question(this, tr("FSMO capture"), tr("Take over the role ") + role_str + "?");

    if (ans == QMessageBox::Yes) {
        FSMORole role = (FSMORole)ui->fsmo_table->item(row, FsmoColumn_Role)->data(ItemRole_FsmoRole).toInt();
        const QString role_dn = dn_from_role(role);
        const AdObject rootDSE = ad.search_object("");
        const QString new_master_service = rootDSE.get_string(ATTRIBUTE_DS_SERVICE_NAME);

        const bool success = ad.attribute_replace_string(role_dn, ATTRIBUTE_FSMO_ROLE_OWNER, new_master_service);
        if (success) {
            btn->setText(tr("Captured"));
            btn->setDisabled(true);
            ui->fsmo_table->item(row, FsmoColumn_Host)->setText(current_dc_dns_name);
            g_status->display_ad_messages(ad, this);
        }
        else {
            g_status->add_message(tr("Failed to capture role") + role_str, StatusType_Error);
            btn->setEnabled(true);
            btn->setText(tr("Capture"));
        }
    }
}
