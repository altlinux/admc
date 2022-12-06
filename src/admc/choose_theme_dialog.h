#ifndef CHOOSETHEMEDIALOG_H
#define CHOOSETHEMEDIALOG_H

#include <QDialog>
#include <QStringListModel>
#include <QDebug>
namespace Ui {
    class ChooseThemeDialog;
}

class ChooseThemeDialog : public QDialog
{
    Q_OBJECT

public:
    ChooseThemeDialog(QWidget * parent);
    Ui::ChooseThemeDialog *ui;

private slots:
    void accept() override;

private:
    QStringListModel * theme_model;
    const QString app_theme_path = ":/icons";
    const QString system_icons_dir_path = "/usr/share/icons";
    QString user_icons_dir_path;

    const QString theme_name_regex_pattern = "Name=.*";
    const QString theme_file_name = "index.theme";

    void setup_theme_list();
    void apply_theme(QString theme_name);
    QStringList get_available_theme_list();
    QStringList parse_theme_names(const QStringList & theme_path_list);
    QStringList get_themes_from_dir(QString path_to_dir);
};

#endif // CHOOSETHEMEDIALOG_H
