#include "ui/widget/tab/sites_link/part_widget.h"
#include "ui/widget/tab/sites_link/ui_part_widget.h"

SitesLinkPartWidget::SitesLinkPartWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SitesLinkPartWidget) {

    ui->setupUi(this);
}

SitesLinkPartWidget::~SitesLinkPartWidget() {
    delete ui;
}

QSpinBox *SitesLinkPartWidget::cost_spinbox() {
    return ui->cost_spbox;
}

QSpinBox *SitesLinkPartWidget::replicate_spinbox() {
    return ui->replicate_spbox;
}

QPushButton *SitesLinkPartWidget::schedule_button() {
    return ui->schedule_button;
}

void SitesLinkPartWidget::retranslate_ui() {
    ui->retranslateUi(this);
    for (auto* widget : children()) {
        QEvent languageEvent(QEvent::LanguageChange);
        QCoreApplication::sendEvent(widget, &languageEvent);
    }
}

bool SitesLinkPartWidget::event(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
    return QObject::event(event);
}
