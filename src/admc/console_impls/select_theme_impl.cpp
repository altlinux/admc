#include <settings.h>
#include "select_theme_impl.h"
#include <QRegularExpression>
#include <QFile>
#include <QIcon>
#include <utils.h>
#include <qcoreapplication.h>
#include <qstandardpaths.h>
#include <qfiledialog.h>
#include <QDebug>
#include <QSettings>
#include <QApplication>

SelectThemeImpl::SelectThemeImpl(ConsoleWidget *console_arg)
    : ConsoleImpl(console_arg)
{
}

QString SelectThemeImpl::read_theme_file()
{
    QString result;

    QString file_path = [&]() {
        const QString caption = QCoreApplication::translate("select_theme_impl.cpp", "Choose app theme");
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        const QString file_filter = QCoreApplication::translate("select_theme_impl.cpp", "System icon theme (*.theme)");
        const QString out = QFileDialog::getOpenFileName(console, caption, dir, file_filter);

        return out;
    }();

    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    return file.readAll();
}

QString SelectThemeImpl::parse_theme_name(QString theme_file_content)
{
    QRegularExpression regex(this->theme_name_regex_pattern);
    auto matched_data = regex.match(theme_file_content, 0, QRegularExpression::PartialPreferCompleteMatch).captured();

    QString result = matched_data.mid(theme_name_regex_pattern.length() - 2);

    return result;
}

QMenu* SelectThemeImpl::format_theme_list(QString active_theme_name, QStringList available_app_theme_names)
{
    auto available_theme_font = QApplication::font();
    available_theme_font.setCapitalization(QFont::Capitalize);

    QMenu app_themes;
    for(int i = 0; i < available_app_theme_names.size(); ++i){
        QString theme_name = available_app_theme_names.at(i);
        QAction theme;

        if (theme_name != active_theme_name) {
            theme.setData(theme_name);
            theme.setIconText(theme_name.toLower());
            theme.setFont(available_theme_font);
        }
        else {
            theme.setData(active_theme_name);
            theme.setIconText(active_theme_name);

            auto active_theme_font = QApplication::font();
            active_theme_font.setBold(true);
            theme.setFont(active_theme_font);
        }

        app_themes.addAction(& theme);
    }

    return &app_themes;
}

void SelectThemeImpl::add_theme_to_list(QString theme_name)
{
    auto app_themes = settings_get_variant(SETTING_app_themes).toList();
    app_themes.append(theme_name);

    settings_set_variant(SETTING_app_themes, app_themes);
}

void SelectThemeImpl::apply_theme(QString theme_name)
{
    QIcon::setThemeName(theme_name);
    settings_set_variant(SETTING_app_active_theme, theme_name);
}

void SelectThemeImpl::apply_default_theme()
{
    QIcon::setThemeName(default_theme_name);
}

QString SelectThemeImpl::add_new_theme()
{
    QString theme_name = parse_theme_name(read_theme_file());
    add_theme_to_list(theme_name);
    apply_theme(theme_name);

    return theme_name;
}
