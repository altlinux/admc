#ifndef SITES_LINK_GENERAL_TAB_H
#define SITES_LINK_GENERAL_TAB_H

#include <QWidget>
#include "ui/widget/tab/sites_link/type.h"

namespace Ui {
class SitesLinkGeneralTab;
}

class AttributeEdit;
class SitesLinkWidget;

class SitesLinkGeneralTab : public QWidget {
    Q_OBJECT

public:
    explicit SitesLinkGeneralTab(QList<AttributeEdit *> *edit_list,
                                 SitesLinkType type,
                                 QWidget *parent = nullptr);
    ~SitesLinkGeneralTab();

    void retranslate_ui();
    bool event(QEvent *event) override;

private:
    Ui::SitesLinkGeneralTab *ui;
    SitesLinkWidget *sites_link_widget = nullptr;
};

#endif // SITES_LINK_GENERAL_TAB_H
