#ifndef PSO_RESULTS_WIDGET_H
#define PSO_RESULTS_WIDGET_H

#include <QWidget>

namespace Ui {
class PSOResultsWidget;
}

class PSOResultsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PSOResultsWidget(QWidget *parent = nullptr);
    ~PSOResultsWidget();

private:
    Ui::PSOResultsWidget *ui;
};

#endif // PSO_RESULTS_WIDGET_H
