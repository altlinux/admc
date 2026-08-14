#ifndef SUBNET_EDIT_WIDGET_H
#define SUBNET_EDIT_WIDGET_H

#include <QWidget>
#include <QHash>

namespace Ui {
class SubnetEditWidget;
}

class AdObject;
class QComboBox;

class SubnetEditWidget : public QWidget {
    Q_OBJECT

public:
    explicit SubnetEditWidget(QWidget *parent = nullptr);
    ~SubnetEditWidget();

    void update(const AdObject &subnet_obj, QHash<QString, QString> sites_dn_name_map);
    void set_read_only(bool read_only);
    QComboBox *subnet_sites_cmbbox();
    QHash<QString, QString> attr_string_values();

    void retranslate_ui();
    bool event(QEvent *event);

private:
    Ui::SubnetEditWidget *ui;
};

#endif // SUBNET_EDIT_WIDGET_H
