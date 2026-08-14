#ifndef GENERAL_SITE_TAB_H
#define GENERAL_SITE_TAB_H

#include <QWidget>

namespace Ui {
class GeneralSiteTab;
}

class AttributeEdit;

class GeneralSiteTab : public QWidget {
    Q_OBJECT

public:
    explicit GeneralSiteTab(QList<AttributeEdit *> *edit_list, QWidget *parent = nullptr);
    ~GeneralSiteTab();

    void retranslate_ui();
    bool event(QEvent *event) override;

private:
    Ui::GeneralSiteTab *ui;
};

#endif // GENERAL_SITE_TAB_H
