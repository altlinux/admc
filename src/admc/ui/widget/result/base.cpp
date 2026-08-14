#include "ui/widget/result/base.h"
#include "ui/widget/result/ui_base.h"

ResultsWidgetBase::ResultsWidgetBase(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResultsWidgetBase) {
    ui->setupUi(this);

    connect(ui->apply_button, &QPushButton::clicked, this, &ResultsWidgetBase::on_apply);
    connect(ui->cancel_button, &QPushButton::clicked, this, &ResultsWidgetBase::on_cancel_edit);
    connect(ui->edit_button, &QPushButton::clicked, this, &ResultsWidgetBase::on_edit);
}

ResultsWidgetBase::~ResultsWidgetBase() {
    delete ui;
}

void ResultsWidgetBase::update(const QModelIndex &index) {
    Q_UNUSED(index)
}

void ResultsWidgetBase::update(const AdObject &obj) {
    Q_UNUSED(obj)
    ui->edit_button->setDisabled(false);
    ui->cancel_button->setDisabled(true);
    ui->apply_button->setDisabled(true);
}

void ResultsWidgetBase::on_apply() {
    return;
}

void ResultsWidgetBase::on_edit() {
    set_editable(true);
}

void ResultsWidgetBase::on_cancel_edit() {
    return;
}

void ResultsWidgetBase::set_editable(bool is_editable) {
    ui->edit_button->setDisabled(is_editable);
    ui->cancel_button->setDisabled(!is_editable);
    ui->apply_button->setDisabled(!is_editable);
}

QStringList ResultsWidgetBase::changed_attrs() {
    return QStringList();
}

void ResultsWidgetBase::retranslate_ui() {
    ui->retranslateUi(this);
}

bool ResultsWidgetBase::event(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
    return QObject::event(event);
}

