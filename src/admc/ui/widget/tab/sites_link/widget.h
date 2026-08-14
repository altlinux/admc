#ifndef SITES_LINK_WIDGET_H
#define SITES_LINK_WIDGET_H

#include <QWidget>
#include "ui/widget/tab/sites_link/type.h"

namespace Ui {
class SitesLinkWidget;
}

class SitesLinkPartWidget;
class SitesLinkCommonWidget;

class SitesLinkWidget : public QWidget {
    Q_OBJECT

public:
    explicit SitesLinkWidget(SitesLinkType type_arg, QWidget *parent = nullptr);
    ~SitesLinkWidget();

    SitesLinkCommonWidget *common_widget();
    SitesLinkPartWidget *sites_link_part_widget();
    SitesLinkType get_type();

    void retranslate_ui();
    bool event(QEvent *event) override;

private:
    Ui::SitesLinkWidget *ui;
    SitesLinkCommonWidget *common_wget = nullptr;
    SitesLinkPartWidget *sites_link_part_wget = nullptr;
    SitesLinkType type;
};

#endif // SITES_LINK_WIDGET_H
