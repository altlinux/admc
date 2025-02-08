#ifndef READWRITEPERMISSIONSWIDGET_H
#define READWRITEPERMISSIONSWIDGET_H

#include "permissions_widget.h"


class ReadWritePermissionsWidget final : public PermissionsWidget {
    Q_OBJECT

public:
    explicit ReadWritePermissionsWidget(QWidget *parent = nullptr);
    ~ReadWritePermissionsWidget();

    virtual void init(const QStringList &target_classes, security_descriptor *sd_arg) override;

private:
    virtual QList<QStandardItem*> create_item_row(const SecurityRight &right) override;
    virtual bool right_applies_to_class(const SecurityRight &right, const QString &obj_class) override;
    virtual bool there_are_rights_for_class(const QString &obj_class) override;
};

#endif // READWRITEPERMISSIONSWIDGET_H
