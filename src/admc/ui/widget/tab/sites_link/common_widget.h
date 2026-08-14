#ifndef SITES_LINK_COMMON_WIDGET_H
#define SITES_LINK_COMMON_WIDGET_H

#include <QWidget>
#include "ui/widget/tab/sites_link/type.h"

namespace Ui {
class SitesLinkCommonWidget;
}

class QLabel;
class QLineEdit;
class QListWidget;
class QIcon;
class QPushButton;

class SitesLinkCommonWidget : public QWidget {
    Q_OBJECT

public:
    explicit SitesLinkCommonWidget(QWidget *parent = nullptr);
    ~SitesLinkCommonWidget();

    QLineEdit *description_line_edit();
    QListWidget *left_list_wget();
    QListWidget *right_list_wget();
    QLabel *left_list_label();
    QLabel *right_list_label();
    QPushButton *add_button();
    QPushButton *remove_button();
    void set_lists_labels(SitesLinkType type);

    void retranslate_ui();
    bool event(QEvent *event) override;

private:
    Ui::SitesLinkCommonWidget *ui;

    void on_add_button();
    void on_remove_button();
    void move_selected_list_items(QListWidget *from_list_wget, QListWidget *to_list_wget);
};

#endif // SITES_LINK_COMMON_WIDGET_H
