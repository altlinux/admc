#ifndef FSMO_TABLE_WIDGET_H
#define FSMO_TABLE_WIDGET_H

#include <QWidget>

namespace Ui {
class FsmoTableWidget;
}

class AdInterface;
class QTableWidgetItem;
class AdObject;

class FsmoTableWidget : public QWidget {
    Q_OBJECT

    enum FsmoColumn {
        FsmoColumn_Role,
        FsmoColumn_Host,
        FsmoColumn_Capture,

        FsmoColumn_COUNT
    };

    enum ItemRole {
        ItemRole_FsmoRole = Qt::UserRole
    };

public:
    explicit FsmoTableWidget(QWidget *parent = nullptr);
    ~FsmoTableWidget();

    void update(AdInterface &ad, const QList<AdObject> &hosts_list);
    void update_role_owner(const QString &new_master_dn, const QString &fsmo_role_dn);

private:
    Ui::FsmoTableWidget *ui;
    QString current_dc_dns_name;
    QHash<QString, QString> dn_dns_name_map; // Contains DC's DN as keys and dns host name as values

    const char *button_property_row = "row"; // capture button property to get its row

    void on_fsmo_capture();
};

#endif // FSMO_TABLE_WIDGET_H
