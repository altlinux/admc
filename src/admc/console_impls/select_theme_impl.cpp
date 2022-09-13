#include "select_theme_impl.h"
#include <QFile>
#include <QIcon>
#include <QDirIterator>
#include <QSettings>
#include <QApplication>
#include <QAction>
#include <QtGlobal>
#include <QRegularExpression>
#include <settings.h>

SelectThemeImpl::SelectThemeImpl()
{
    user_icons_dir_path = QString(QDir::separator()).append("home")
            .append(QDir::separator().toLatin1()).append(qgetenv("USER"))
            .append(QDir::separator().toLatin1()).append(".icons")
            .append(QDir::separator().toLatin1());
}

void SelectThemeImpl::set_theme(const QString& theme_name)
{
    settings_set_variant(SETTING_app_active_theme, theme_name);
    apply_theme(theme_name);
}

void SelectThemeImpl::apply_theme(QString theme_name)
{
    QIcon::setThemeName(theme_name);
    settings_set_variant(SETTING_app_active_theme, theme_name);
}

QStringList SelectThemeImpl::get_themes_from_dir(QString path_to_dir)
{
    QDirIterator it(path_to_dir, QStringList("index.theme"), QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);

    QStringList files;

    while(it.hasNext())
    {
        files << it.next();
    }

    return parse_theme_names(files);
}

QStringList SelectThemeImpl::parse_theme_names(const QStringList & theme_path_list)
{
    QRegularExpression regex(this->theme_name_regex_pattern);

    QStringList result;

    for (int i = 0; i < theme_path_list.size(); ++i)
    {
        QFile theme_file(theme_path_list.at(i));

        if(!theme_file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            continue;
        }

        auto matched_data = regex.match(theme_file.readAll(), 0, QRegularExpression::PartialPreferCompleteMatch).captured();

        result << matched_data.mid(theme_name_regex_pattern.length() - 2);
    }

    return result;
}

QStringList SelectThemeImpl::get_available_theme_list()
{
    auto available_theme_paths = get_themes_from_dir(this->system_icons_dir_path);
    available_theme_paths.append(get_themes_from_dir(this->user_icons_dir_path));
    available_theme_paths.append(get_themes_from_dir(this->app_theme_path));

    return available_theme_paths;
}
