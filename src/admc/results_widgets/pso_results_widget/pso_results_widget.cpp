#include "pso_results_widget.h"
#include "ui_pso_results_widget.h"

PSOResultsWidget::PSOResultsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PSOResultsWidget)
{
    ui->setupUi(this);
}

PSOResultsWidget::~PSOResultsWidget()
{
    delete ui;
}
