#ifndef SUBNET_RESULTS_WIDGET_H
#define SUBNET_RESULTS_WIDGET_H

#include "ui/widget/result/base.h"

class SubnetEditWidget;

class SubnetResultsWidget final : public ResultsWidgetBase {
    Q_OBJECT

public:
    explicit SubnetResultsWidget(QWidget *parent = nullptr);
    virtual ~SubnetResultsWidget() = default;

    virtual void update(const AdObject &obj);

private:
    SubnetEditWidget *subnet_edit_wget;
    QHash<QString, QString> site_dn_name_map;

    virtual void on_apply() override;
    virtual void on_edit() override;
    virtual void on_cancel_edit() override;
    virtual void set_editable(bool is_editable) override;
    virtual QStringList changed_attrs() override;
};

#endif // SUBNET_RESULTS_WIDGET_H
