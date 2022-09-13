#ifndef QUERY_THEME_IMPL_H
#define QUERY_THEME_IMPL_H
#include <QString>
#include <QMenu>
#include <QDir>
#include <console_widget/console_impl.h>

class SelectThemeImpl
{
public:
    SelectThemeImpl();
    void apply_theme(QString theme_name);
    QStringList get_available_theme_list();

private slots:
    void set_theme(const QString& themeName);

private:
    QStringList parse_theme_names(const QStringList & theme_path_list);
    QStringList get_themes_from_dir(QString path_to_dir);

    QMenu * menu_theme;

    const QString app_theme_path = ":/icons";
    const QString system_icons_dir_path = "/usr/share/icons";
    QString user_icons_dir_path;

    const QString theme_name_regex_pattern = "Name=.*";
};

#endif // QUERY_THEME_IMPL_H
