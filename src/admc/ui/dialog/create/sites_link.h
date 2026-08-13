#ifndef SITES_LINK_CREATION_DIALOG_H
#define SITES_LINK_CREATION_DIALOG_H

#include "ui/dialog/create/object.h"
#include "tabs/sites_link_tab/sites_link_type.h"

namespace Ui {
class CreateSitesLinkDialog;
}

class AdInterface;

class CreateSitesLinkDialog : public CreateObjectDialog {
    Q_OBJECT

public:
    explicit CreateSitesLinkDialog(AdInterface &ad, SitesLinkType type_arg, const QString parent_dn_arg, QWidget *parent = nullptr);
    ~CreateSitesLinkDialog();

    void accept() override;
    QString get_created_dn() const override;

private:
    Ui::CreateSitesLinkDialog *ui;
    SitesLinkType type;
    QString created_dn;
    QString parent_dn;
};

#endif // SITES_LINK_CREATION_DIALOG_H
