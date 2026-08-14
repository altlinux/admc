#include "ui/widget/tab/sites_link/common_widget.h"
#include "ui/widget/tab/sites_link/ui_common_widget.h"

SitesLinkCommonWidget::SitesLinkCommonWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SitesLinkCommonWidget) {
    ui->setupUi(this);

    connect(ui->add_button, &QPushButton::clicked, this, &SitesLinkCommonWidget::on_add_button);
    connect(ui->remove_button, &QPushButton::clicked, this, &SitesLinkCommonWidget::on_remove_button);
}

SitesLinkCommonWidget::~SitesLinkCommonWidget() {
    delete ui;
}

QLineEdit *SitesLinkCommonWidget::description_line_edit() {
    return ui->description_edit;
}

QListWidget *SitesLinkCommonWidget::left_list_wget() {
    return ui->left_list_wget;
}

QListWidget *SitesLinkCommonWidget::right_list_wget() {
    return ui->right_list_wget;
}

QLabel *SitesLinkCommonWidget::left_list_label() {
    return ui->left_list_label;
}

QLabel *SitesLinkCommonWidget::right_list_label() {
    return ui->right_list_label;
}

QPushButton *SitesLinkCommonWidget::add_button() {
    return ui->add_button;
}

QPushButton *SitesLinkCommonWidget::remove_button() {
    return ui->remove_button;
}

void SitesLinkCommonWidget::set_lists_labels(SitesLinkType type) {
    if (type == SitesLinkType::Link) {
        ui->left_list_label->setText(tr("Sites not in this site link"));
        ui->right_list_label->setText(tr("Sites in this site link"));
    } else {
        ui->left_list_label->setText(tr("Site links not included in this bridge"));
        ui->right_list_label->setText(tr("Site links included in this bridge"));
    }
}

void SitesLinkCommonWidget::on_add_button() {
    move_selected_list_items(ui->left_list_wget, ui->right_list_wget);
}

void SitesLinkCommonWidget::on_remove_button() {
    move_selected_list_items(ui->right_list_wget, ui->left_list_wget);
}

void SitesLinkCommonWidget::move_selected_list_items(QListWidget *from_list_wget, QListWidget *to_list_wget) {
    auto items = from_list_wget->selectedItems();
    if (items.isEmpty()) {
        return;
    }

    for (auto item : items) {
        int taken_row = from_list_wget->row(item);
        to_list_wget->addItem(from_list_wget->takeItem(taken_row));
    }
}

void SitesLinkCommonWidget::retranslate_ui() {
    ui->retranslateUi(this);
    for (auto* widget : children()) {
        QEvent languageEvent(QEvent::LanguageChange);
        QCoreApplication::sendEvent(widget, &languageEvent);
    }
}

bool SitesLinkCommonWidget::event(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
    return QObject::event(event);
}

