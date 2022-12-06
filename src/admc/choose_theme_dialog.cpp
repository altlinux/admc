#include "choose_theme_dialog.h"
#include "ui_choose_theme_dialog.h"
#include <settings.h>
#include <QFile>
#include <QDirIterator>


ChooseThemeDialog::ChooseThemeDialog(QWidget *parent):
    QDialog(parent)
{
    ui = new Ui::ChooseThemeDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    setup_theme_list();
}

void ChooseThemeDialog::setup_theme_list()
{
    auto themes = get_available_theme_list();

    theme_model = new QStringListModel(this);

    theme_model->setStringList(themes);

    ui->theme_list->setModel(theme_model);
}

void ChooseThemeDialog::accept()
{
    apply_theme(ui->theme_list->currentIndex().data().toString());
    QDialog::accept();
}

void ChooseThemeDialog::apply_theme(QString theme_name)
{
    QIcon::setThemeName(theme_name);
    settings_set_variant(SETTING_app_active_theme, theme_name);
}

QStringList ChooseThemeDialog::get_themes_from_dir(QString path_to_dir)
{
    QDirIterator it(path_to_dir, QStringList(theme_file_name), QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);

    QStringList files;

    while(it.hasNext())
    {
        files << it.next();
    }

    return parse_theme_names(files);
}

QStringList ChooseThemeDialog::parse_theme_names(const QStringList & theme_path_list)
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

QStringList ChooseThemeDialog::get_available_theme_list()
{
    auto available_theme_paths = get_themes_from_dir(this->system_icons_dir_path);
    available_theme_paths.append(get_themes_from_dir(this->user_icons_dir_path));
    available_theme_paths.append(get_themes_from_dir(this->app_theme_path));

    return available_theme_paths;
}
