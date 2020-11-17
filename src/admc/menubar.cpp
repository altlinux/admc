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
#include "confirmation_dialog.h"
#include "find_dialog.h"
#include "ad_interface.h"

#include <QMenu>
#include <QLocale>
#include <QMessageBox>
#include <QActionGroup>
#include <QApplication>

MenuBar::MenuBar(QWidget* parent)
: QMenuBar(parent) {
    QMenu *menubar_action = addMenu(tr("Action"));

    menubar_action->addAction(tr("Find"),
        []() {
            auto find_dialog = new FindDialog();
            find_dialog->open();
        });

    menubar_action->addAction(tr("Refresh"),
        []() {
            AD()->refresh();
        });

    QAction *exit_action = menubar_action->addAction(tr("Exit"));
    connect(
        exit_action, &QAction::triggered,
        this, &MenuBar::on_exit_action);

    auto add_bool_setting_action = 
    [](QMenu *menu, QString display_text, BoolSetting type) {
        QAction *action = menu->addAction(display_text);
        SETTINGS()->connect_action_to_bool_setting(action, type);
    };

    QMenu *menubar_view = addMenu(tr("View"));
    add_bool_setting_action(menubar_view, tr("Advanced view"), BoolSetting_AdvancedView);
    add_bool_setting_action(menubar_view, tr("Show status log"), BoolSetting_ShowStatusLog);
    add_bool_setting_action(menubar_view, tr("Dock Details dialog"), BoolSetting_DetailsIsDocked);

    QMenu *menubar_preferences = addMenu(tr("Preferences"));
    add_bool_setting_action(menubar_preferences, tr("Confirm actions"), BoolSetting_ConfirmActions);
    add_bool_setting_action(menubar_preferences, tr("Dev mode"), BoolSetting_DevMode);
    add_bool_setting_action(menubar_preferences, tr("Show non-container objects in Containers tree"), BoolSetting_ShowNonContainersInContainersTree);
    add_bool_setting_action(menubar_preferences, tr("Put last name before first name when creating users"), BoolSetting_LastNameBeforeFirstName);

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

        const QLocale saved_locale = SETTINGS()->get_variant(VariantSetting_Locale).toLocale();
        const QLocale::Language saved_language = saved_locale.language();
        if (language == saved_language) {
            action->setChecked(true);
        }

        connect(
            action, &QAction::toggled,
            [this, locale](bool checked) {
                if (checked) {
                    SETTINGS()->set_variant(VariantSetting_Locale, locale);

                    QMessageBox::information(this, tr("Info"), tr("App needs to be restarted for the language option to take effect."));
                }
            }
            );
    };

    add_language_action(QLocale::English);
    add_language_action(QLocale::Russian);
}

void MenuBar::on_exit_action() {
    const QString text = QString(tr("Are you sure you want to exit?"));
    const bool confirmed = confirmation_dialog(text, this);

    if (confirmed) {
        QApplication::quit();
    }   
}
