#include "ui/widget/tab/sites_link/general_tab.h"
#include "ui/widget/tab/sites_link/ui_general_tab.h"
#include "ui/widget/tab/sites_link/widget.h"
#include "attribute_edits/sites_link_edit.h"
#include "attribute_edits/general_name_edit.h"
#include "attribute_edits/schedule_hours_edit.h"
#include "ui/widget/tab/sites_link/part_widget.h"

SitesLinkGeneralTab::SitesLinkGeneralTab(QList<AttributeEdit *> *edit_list, SitesLinkType type, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SitesLinkGeneralTab),
    sites_link_widget(new SitesLinkWidget(type, this)) {

    ui->setupUi(this);

    edit_list->append(new GeneralNameEdit(ui->name_label, this));
    edit_list->append(new SitesLinkEdit(sites_link_widget, this));

    if (sites_link_widget->sites_link_part_widget()) {
        QPushButton *schedule_button = sites_link_widget->sites_link_part_widget()->schedule_button();
        edit_list->append(new ScheduleHoursEdit(schedule_button, this));
    }

    ui->verticalLayout->addWidget(sites_link_widget);
}

SitesLinkGeneralTab::~SitesLinkGeneralTab() {
    delete ui;
}

void SitesLinkGeneralTab::retranslate_ui() {
    ui->retranslateUi(this);
    for (auto* widget : children()) {
        QEvent languageEvent(QEvent::LanguageChange);
        QCoreApplication::sendEvent(widget, &languageEvent);
    }
}

bool SitesLinkGeneralTab::event(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
    return QObject::event(event);
}
