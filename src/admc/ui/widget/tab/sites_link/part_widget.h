#ifndef SITES_LINK_PART_WIDGET_H
#define SITES_LINK_PART_WIDGET_H

#include <QWidget>

namespace Ui {
class SitesLinkPartWidget;
}

class AdObject;
class QSpinBox;
class QPushButton;

class SitesLinkPartWidget : public QWidget {
    Q_OBJECT

public:
    explicit SitesLinkPartWidget(QWidget *parent = nullptr);
    ~SitesLinkPartWidget();

     QSpinBox *cost_spinbox();
     QSpinBox *replicate_spinbox();
     QPushButton *schedule_button();

    void retranslate_ui();
    bool event(QEvent *event) override;

private:
    Ui::SitesLinkPartWidget *ui;
};

#endif // SITES_LINK_PART_WIDGET_H
