#ifndef CHOOSETHEMEDIALOG_H
#define CHOOSETHEMEDIALOG_H

#include <QDialog>
#include <QStringListModel>
namespace Ui {
    class ChooseThemeDialog;
}

class ChooseThemeDialog : public QDialog
{
    Q_OBJECT
public:
    ChooseThemeDialog(QWidget * parent);
    Ui::ChooseThemeDialog *ui;
    void setup_theme_list();
    void apply_theme();

private slots:
    void accepted();

private:
    QStringListModel * theme_model;
};

#endif // CHOOSETHEMEDIALOG_H
