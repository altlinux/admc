#ifndef QUERY_THEME_IMPL_H
#define QUERY_THEME_IMPL_H
#include <QString>
#include <console_widget/console_impl.h>

class SelectThemeImpl : public ConsoleImpl
{
    Q_OBJECT
public:
    SelectThemeImpl(ConsoleWidget *console_arg);
    void apply_theme(QString theme_name);
    void apply_default_theme();
    QString add_new_theme();
    QMenu* format_theme_list(QString active_theme_name, QStringList available_app_themes);
private:
    QString read_theme_file();
    QString parse_theme_name(QString theme_file_name);
    void add_theme_to_list(QString theme_name);

    const QString theme_name_regex_pattern = "Name=.*";
    const QString default_theme_name = "admc-icons";
};

#endif // QUERY_THEME_IMPL_H
