/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "menubar.h"
#include "settings.h"
#include "login_dialog.h"
#include "confirmation_dialog.h"

#include <QMenu>
#include <QLocale>
#include <QMessageBox>
#include <QActionGroup>
#include <QApplication>

MenuBar::MenuBar(QWidget* parent)
: QMenuBar(parent) {
    login_dialog = new LoginDialog(this);

    QMenu *menubar_file = addMenu(tr("File"));

    QAction *login_action = menubar_file->addAction(tr("Login"));
    connect(
        login_action, &QAction::triggered,
        this, &MenuBar::on_login_action);

    QAction *exit_action = menubar_file->addAction(tr("Exit"));
    connect(
        exit_action, &QAction::triggered,
        this, &MenuBar::on_exit_action);

    auto add_bool_setting_action = 
    [](QMenu *menu, QString display_text, BoolSetting type) {
        QAction *action = menu->addAction(display_text);
        Settings::instance()->connect_action_to_get_bool_signal(action, type);
    };

    QMenu *menubar_view = addMenu(tr("View"));
    add_bool_setting_action(menubar_view, tr("Advanced view"), BoolSetting_AdvancedView);
    add_bool_setting_action(menubar_view, tr("Show DN column"), BoolSetting_DnColumn);
    add_bool_setting_action(menubar_view, tr("Show status log"), BoolSetting_ShowStatusLog);

    QMenu *menubar_preferences = addMenu(tr("Preferences"));
    add_bool_setting_action(menubar_preferences, tr("Open attributes on left click in Containers window"), BoolSetting_DetailsFromContainers);
    add_bool_setting_action(menubar_preferences, tr("Open attributes on left click in Contents window"), BoolSetting_DetailsFromContents);
    add_bool_setting_action(menubar_preferences, tr("Confirm actions"), BoolSetting_ConfirmActions);
    add_bool_setting_action(menubar_preferences, tr("Login using saved session at startup"), BoolSetting_AutoLogin);

    QMenu *language_menu = menubar_preferences->addMenu(tr("Language"));
    auto language_group = new QActionGroup(language_menu);

    auto add_language_action =
    [this, language_menu, language_group] (QLocale::Language language) {
        QLocale locale(language);
        const QString language_name = locale.nativeLanguageName();

        const auto action = new QAction(language_name, language_group);
        action->setCheckable(true);
        language_group->addAction(action);
        language_menu->addAction(action);

        const QLocale saved_locale = Settings::instance()->get_variant(VariantSetting_Locale).toLocale();
        const QLocale::Language saved_language = saved_locale.language();
        if (language == saved_language) {
            action->setChecked(true);
        }

        connect(
            action, &QAction::toggled,
            [this, locale](bool checked) {
                if (checked) {
                    Settings::instance()->set_variant(VariantSetting_Locale, locale);

                    QMessageBox::information(this, tr("Info"), tr("App needs to be restarted for the language option to take effect."));
                }
            }
            );
    };

    add_language_action(QLocale::English);
    add_language_action(QLocale::Russian);
}

void MenuBar::on_login_action() {
    login_dialog->open();
    const QString text = QString(tr("Are you sure you want to exit?"));
    const bool confirmed = confirmation_dialog(text, this);

    if (confirmed) {
        QApplication::closeAllWindows();
        QApplication::quit();
    }   
}

void MenuBar::on_exit_action() {
    const QString text = QString(tr("Are you sure you want to exit?"));
    const bool confirmed = confirmation_dialog(text, this);

    if (confirmed) {
        QApplication::closeAllWindows();
        QApplication::quit();
    }   
}
