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

#include "main_window.h"
#include "status.h"
#include "settings.h"
#include "ad/ad_interface.h"
#include "ad/ad_config.h"
#include "console.h"
#include "console_widget/console_widget.h"
#include "toggle_widgets_dialog.h"
#include "about_dialog.h"
#include "manual_dialog.h"

#include <QApplication>
#include <QString>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QAction>
#include <QDesktopWidget>
#include <QMenuBar>
#include <QActionGroup>
#include <QMessageBox>

MainWindow::MainWindow()
: QMainWindow()
{
    if (SETTINGS()->contains_variant(VariantSetting_MainWindowGeometry)) {
        SETTINGS()->restore_geometry(this, VariantSetting_MainWindowGeometry);
    } else {
        // Make window 70% of desktop size for first startup
        resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
    }

    QStatusBar *status_bar = STATUS()->status_bar;
    setStatusBar(status_bar);

    STATUS()->status_bar->showMessage(tr("Ready"));
    SETTINGS()->connect_toggle_widget(STATUS()->status_log, BoolSetting_ShowStatusLog);

    console = new Console();

    setup_menubar();

    auto vert_splitter = new QSplitter(Qt::Vertical);
    vert_splitter->addWidget(STATUS()->status_log);
    vert_splitter->addWidget(console);
    vert_splitter->setStretchFactor(0, 1);
    vert_splitter->setStretchFactor(1, 3);

    setCentralWidget(vert_splitter);

    connect_to_server();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    SETTINGS()->save_geometry(this, VariantSetting_MainWindowGeometry);

    QApplication::quit();
}

void MainWindow::setup_menubar() {
    auto menubar = new QMenuBar();
    setMenuBar(menubar);

    // Create dialogs opened from menubar
    auto manual_dialog = new ManualDialog(this);
    auto about_dialog = new AboutDialog(this);
    auto toggle_widgets_dialog = new ToggleWidgetsDialog(this);

    //
    // Create actions
    //
    connect_action = new QAction(tr("&Connect"));
    auto quit_action = new QAction(tr("&Quit"));

    auto toggle_widgets_action = new QAction(tr("&Toggle widgets"), this);

    auto manual_action = new QAction(tr("&Manual"), this);
    auto about_action = new QAction(tr("&About ADMC"), this);

    auto confirm_actions_action = new QAction(tr("&Confirm actions"), this);
    auto last_before_first_name_action = new QAction(tr("&Put last name before first name when creating users"), this);

    const QList<QLocale::Language> language_list = {
        QLocale::English,
        QLocale::Russian,
    };
    const QHash<QLocale::Language, QAction *> language_actions =
    [this, language_list]() {
        QHash<QLocale::Language, QAction *> out;

        auto language_group = new QActionGroup(this);
        for (const auto language : language_list) {
            QLocale locale(language);
            const QString language_name =
            [locale]() {
            // NOTE: Russian nativeLanguageName starts with lowercase letter for some reason
                QString name_out = locale.nativeLanguageName();

                const QChar first_letter_uppercased = name_out[0].toUpper();

                name_out.replace(0, 1, first_letter_uppercased);

                return name_out;
            }();

            const auto action = new QAction(language_name, language_group);
            action->setCheckable(true);
            language_group->addAction(action);

            out[language] = action;
        }

        return out;
    }();

    //
    // Create menus
    //
    // NOTE: for menu's that are obtained from console, we
    // don't add actions. Instead the console adds actions
    // to them.
    auto file_menu = menubar->addMenu(tr("&File"));
    menubar->addMenu(console->console_widget->get_action_menu());
    menubar->addMenu(console->console_widget->get_navigation_menu());
    menubar->addMenu(console->console_widget->get_view_menu());
    auto preferences_menu = menubar->addMenu(tr("&Preferences"));
    auto language_menu = new QMenu(tr("&Language"));
    auto help_menu = menubar->addMenu(tr("&Help"));

    //
    // Fill menus
    //
    file_menu->addAction(connect_action);
    file_menu->addAction(quit_action);

    preferences_menu->addAction(confirm_actions_action);
    preferences_menu->addAction(last_before_first_name_action);
    preferences_menu->addAction(toggle_widgets_action);
    preferences_menu->addMenu(language_menu);

    for (const auto language : language_list) {
        QAction *language_action = language_actions[language];
        language_menu->addAction(language_action);
    }

    help_menu->addAction(manual_action);
    help_menu->addAction(about_action);
    
    //
    // Connect actions
    //

    connect(
        connect_action, &QAction::triggered,
        this, &MainWindow::connect_to_server);
    connect(
        quit_action, &QAction::triggered,
        this, &MainWindow::quit);
    connect(
        manual_action, &QAction::triggered,
        manual_dialog, &QDialog::open);
    connect(
        about_action, &QAction::triggered,
        about_dialog, &QDialog::open);
    connect(
        toggle_widgets_action, &QAction::triggered,
        toggle_widgets_dialog, &QDialog::open);
    SETTINGS()->connect_action_to_bool_setting(confirm_actions_action, BoolSetting_ConfirmActions);
    SETTINGS()->connect_action_to_bool_setting(last_before_first_name_action, BoolSetting_LastNameBeforeFirstName);

    for (const auto language : language_actions.keys()) {
        QAction *action = language_actions[language];

        connect(
            action, &QAction::toggled,
            [this, language](bool checked) {
                if (checked) {
                    SETTINGS()->set_variant(VariantSetting_Locale, QLocale(language));

                    QMessageBox::information(this, tr("Info"), tr("App needs to be restarted for the language option to take effect."));
                }
            });
    }
}

void MainWindow::connect_to_server() {
    AdInterface ad;
    if (ad_connected(ad)) {
        // TODO: check for load failure
        const QLocale locale = SETTINGS()->get_variant(VariantSetting_Locale).toLocale();
        ADCONFIG()->load(ad, locale);

        STATUS()->display_ad_messages(ad, this);

        console->go_online(ad);
        connect_action->setEnabled(false);
    }
}

void MainWindow::quit() {
    QApplication::quit();
}
