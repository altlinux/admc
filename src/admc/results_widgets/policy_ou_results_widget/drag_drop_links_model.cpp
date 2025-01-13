#include "drag_drop_links_model.h"
#include "utils.h"
#include "globals.h"
#include "managers/icon_manager.h"
#include "ad_interface.h"
#include "console_impls/policy_impl.h"

#include <QMimeData>
#include <QDataStream>
#include <QMessageBox>
#include <QDebug>

DragDropLinksModel::DragDropLinksModel(const Gplink &gplink, int rows, int columns, QObject *parent)
    : QStandardItemModel{rows, columns, parent}, gplink_ref{gplink} {

}

QMimeData *DragDropLinksModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mime_data = new QMimeData;
    QByteArray encoded_data;
    QDataStream stream(&encoded_data, QIODevice::WriteOnly);

    int row = -1;
    for (const QModelIndex &index : indexes) {
        if (row != index.row()) {
        row = index.row();

        // Data for items to be inserted after drop is taken from gplink object. But gplink doesn't
        // contain visible name, thus visible name is also added to mime data
        const QString dn_name_data_string = index.data(LinkedPoliciesRole_DN).toString() + ':' +
                index.siblingAtColumn(LinkedPoliciesColumn_Name).data().toString();
        stream << dn_name_data_string;
        }
    }

    mime_data->setData(links_model_mime_type, encoded_data);
    return mime_data;
}

Qt::DropActions DragDropLinksModel::supportedDropActions() const {
    return Qt::MoveAction;
}

bool DragDropLinksModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if (!canDropMimeData(data, action, row, column, parent)) {
        return false;
    }

    int insert_row;
    if (row != -1) {
        insert_row = row;
    }
    else if (parent.isValid()) {
        insert_row = parent.row();
    }
    else {
        insert_row = rowCount();
    }

    QByteArray encoded_data = data->data(links_model_mime_type);
    QDataStream stream(&encoded_data, QIODevice::ReadOnly);
    QStringList dn_name_strings;
    while (!stream.atEnd()) {
        QString dn_name;
        stream >> dn_name;
        dn_name_strings << dn_name;
    }

    const int max_order = gplink_ref.get_max_order();
    int target_order;
    if (insert_row == rowCount()) {
        target_order = max_order;
    }
    else {
        target_order = index(insert_row, LinkedPoliciesColumn_Order).data().toInt();
    }

    Gplink gplink_modified = gplink_ref;

    for (const QString &dn_name : dn_name_strings) {
        const QString dn = dn_name.split(':').first();
        const QString display_name = dn_name.split(':').last();
        int current_order = gplink_ref.get_gpo_order(dn);

        QList<QStandardItem *> item_row = make_item_row(LinkedPoliciesColumn_COUNT);
        load_row(item_row, current_order, dn, display_name);
        insertRow(insert_row, item_row);

        gplink_modified.move(current_order, target_order);

        if (insert_row < rowCount()) {
            insert_row++;
        }
        if (target_order < max_order) {
            target_order++;
        }
    }

    arrange_orders_from_gplink(gplink_modified);

    emit link_orders_changed(gplink_modified);

    return true;
}

bool DragDropLinksModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent)

    if (!data->hasFormat(links_model_mime_type)) {
        return false;
    }

    return true;
}

void DragDropLinksModel::arrange_orders_from_gplink(const Gplink &gplink) {
    for (int row = 0; row < rowCount(); ++row) {
        QStandardItem *link_item = item(row, LinkedPoliciesColumn_Order);
        const QString gpo_dn = link_item->data(LinkedPoliciesRole_DN).toString();
        const int previous_order = link_item->data(Qt::DisplayRole).toInt();
        const int actual_order = gplink.get_gpo_order(gpo_dn);
        if (previous_order != actual_order) {
            link_item->setData(actual_order, Qt::DisplayRole);
        }
    }
}

void DragDropLinksModel::update_sort_column(LinkedPoliciesColumn column) {
    sort_column = column;
}

void DragDropLinksModel::load_row(QList<QStandardItem *> &item_row, int order, const QString &dn, const QString &display_name) {
    Qt::CheckState is_enforced = gplink_ref.get_option(dn, GplinkOption_Enforced) ? Qt::Checked : Qt::Unchecked;
    Qt::CheckState is_disabled = gplink_ref.get_option(dn, GplinkOption_Disabled) ? Qt::Checked : Qt::Unchecked;

    set_policy_link_icon(item_row[0], is_enforced == Qt::Checked, is_disabled == Qt::Checked);
    item_row[LinkedPoliciesColumn_Order]->setData(order, Qt::DisplayRole);
    item_row[LinkedPoliciesColumn_Name]->setText(display_name);
    item_row[LinkedPoliciesColumn_Enforced]->setCheckable(true);
    item_row[LinkedPoliciesColumn_Enforced]->setCheckState(is_enforced);
    item_row[LinkedPoliciesColumn_Disabled]->setCheckable(true);
    item_row[LinkedPoliciesColumn_Disabled]->setCheckState(is_disabled);

    set_data_for_row(item_row, dn, LinkedPoliciesRole_DN);
}
