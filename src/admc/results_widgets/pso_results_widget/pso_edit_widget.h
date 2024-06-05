#ifndef PSO_EDIT_WIDGET_H
#define PSO_EDIT_WIDGET_H

#include <QWidget>

namespace Ui {
class PsoEditWidget;
}

class PSOEditWidget final : public QWidget {
    Q_OBJECT

public:
    explicit PSOEditWidget(QWidget *parent = nullptr);
    ~PSOEditWidget();

    QString get_pso_dn() const;
    QStringList get_applied_dn_list() const;

private:
    Ui::PsoEditWidget *ui;

    void load_defaults();
    void on_add();
    void on_remove();
};

#endif // PSO_EDIT_WIDGET_H
