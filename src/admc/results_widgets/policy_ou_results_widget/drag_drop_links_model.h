#ifndef DRAGDROPLINKSMODEL_H
#define DRAGDROPLINKSMODEL_H

#include <QStandardItemModel>
#include <QSet>
#include <QMap>

#include "gplink.h"

template <class T>
class QList;
class QPersistentModelIndex;

enum LinkedPoliciesColumn {
    LinkedPoliciesColumn_Order,
    LinkedPoliciesColumn_Name,
    LinkedPoliciesColumn_Enforced,
    LinkedPoliciesColumn_Disabled,

    LinkedPoliciesColumn_COUNT,
};

enum LinkedPoliciesRole {
    LinkedPoliciesRole_DN = Qt::UserRole + 1,

    LinkedPoliciesRole_COUNT,
};

const QSet<LinkedPoliciesColumn> option_columns = {
    LinkedPoliciesColumn_Enforced,
    LinkedPoliciesColumn_Disabled,
};

const QMap<LinkedPoliciesColumn, GplinkOption> column_to_option = {
    {LinkedPoliciesColumn_Enforced, GplinkOption_Enforced},
    {LinkedPoliciesColumn_Disabled, GplinkOption_Disabled},
};

class DragDropLinksModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit DragDropLinksModel(const Gplink &gplink, int rows, int columns, QObject *parent = nullptr);

    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    virtual Qt::DropActions supportedDropActions() const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

    void arrange_orders_from_gplink(const Gplink &gplink);
    void update_sort_column(LinkedPoliciesColumn column);

signals:
    void link_orders_changed(const Gplink &gplink);

private:
    const QString links_model_mime_type = "application/vnd.text.list";
    const Gplink &gplink_ref;
    LinkedPoliciesColumn sort_column;

    void load_row(QList<QStandardItem*> &item_row, int order, const QString &dn, const QString &display_name);
};

#endif // DRAGDROPLINKSMODEL_H
