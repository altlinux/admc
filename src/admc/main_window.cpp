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
#include "properties_dialog.h"
#include "status.h"
#include "settings.h"
#include "policies_widget.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "object_model.h"
#include "filter_dialog.h"
#include "object_menu.h"
#include "console.h"
#include "utils.h"
#include "toggle_widgets_dialog.h"
#include "config.h"
#include "help_browser.h"

#include <QApplication>
#include <QString>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QAction>
#include <QDesktopWidget>
#include <QMenu>
#include <QLocale>
#include <QMessageBox>
#include <QActionGroup>
#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMenuBar>
#include <QHelpEngine>
#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QTabWidget>
#include <QStandardPaths>

MainWindow::MainWindow()
: QMainWindow()
{
    if (SETTINGS()->contains_variant(VariantSetting_MainWindowGeometry)) {
        SETTINGS()->restore_geometry(this, VariantSetting_MainWindowGeometry);
    } else {
        // Make window 70% of desktop size for first startup
        resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
    }

    console = new Console();

    QStatusBar *status_bar = STATUS()->status_bar;
    setStatusBar(status_bar);

    STATUS()->status_bar->showMessage(tr("Ready"));
    SETTINGS()->connect_toggle_widget(STATUS()->status_log, BoolSetting_ShowStatusLog);

    auto vert_splitter = new QSplitter(Qt::Vertical);
    vert_splitter->addWidget(STATUS()->status_log);
    vert_splitter->addWidget(console);
    vert_splitter->setStretchFactor(0, 1);
    vert_splitter->setStretchFactor(1, 3);

    setCentralWidget(vert_splitter);

    setMenuBar(new QMenuBar());

    //
    // Create actions
    //
    connect_action = new QAction(tr("&Connect"));
    auto quit_action = new QAction(tr("&Quit"));

    auto advanced_view_action = new QAction(tr("&Advanced View"), this);
    auto toggle_widgets_action = new QAction(tr("&Toggle widgets"), this);

    auto manual_action = new QAction(tr("&Manual"), this);
    auto about_action = new QAction(tr("&About ADMC"), this);

    auto dev_mode_action = new QAction(tr("Dev mode"), this);
    auto confirm_actions_action = new QAction(tr("&Confirm actions"), this);
    auto show_noncontainers_action = new QAction(tr("&Show non-container objects in Console tree"), this);
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
    auto file_menu = menuBar()->addMenu(tr("&File"));
    auto action_menu = menuBar()->addMenu(tr("&Action"));
    auto navigation_menu = menuBar()->addMenu(tr("&Navigation"));
    auto view_menu = menuBar()->addMenu(tr("&View"));
    auto preferences_menu = menuBar()->addMenu(tr("&Preferences"));
    auto language_menu = new QMenu(tr("&Language"));
    auto help_menu = menuBar()->addMenu(tr("&Help"));

    //
    // Fill menus
    //
    file_menu->addAction(connect_action);
    file_menu->addAction(quit_action);

    navigation_menu->addAction(console->get_navigate_up_action());
    navigation_menu->addAction(console->get_navigate_back_action());
    navigation_menu->addAction(console->get_navigate_forward_action());

    view_menu->addAction(advanced_view_action);
    view_menu->addAction(toggle_widgets_action);
    view_menu->addAction(console->get_open_filter_action());

    #ifdef QT_DEBUG
    preferences_menu->addAction(dev_mode_action);
    #endif
    preferences_menu->addAction(confirm_actions_action);
    preferences_menu->addAction(show_noncontainers_action);
    preferences_menu->addAction(last_before_first_name_action);
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
        this, &MainWindow::manual);
    connect(
        about_action, &QAction::triggered,
        this, &MainWindow::about);
    connect(
        toggle_widgets_action, &QAction::triggered,
        this, &MainWindow::open_toggle_widgets_dialog);
    SETTINGS()->connect_action_to_bool_setting(advanced_view_action, BoolSetting_AdvancedView);
    SETTINGS()->connect_action_to_bool_setting(dev_mode_action, BoolSetting_DevMode);
    SETTINGS()->connect_action_to_bool_setting(confirm_actions_action, BoolSetting_ConfirmActions);
    SETTINGS()->connect_action_to_bool_setting(show_noncontainers_action, BoolSetting_ShowNonContainersInConsoleTree);
    SETTINGS()->connect_action_to_bool_setting(last_before_first_name_action, BoolSetting_LastNameBeforeFirstName);
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

    connect(
        action_menu, &QMenu::aboutToShow,
        [this, action_menu]() {
            console->load_menu(action_menu);
        });

    connect_to_server();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    SETTINGS()->save_geometry(this, VariantSetting_MainWindowGeometry);

    QApplication::quit();
}

void MainWindow::connect_to_server() {
    AdInterface ad;
    if (ad_connected(ad)) {
        // TODO: check for load failure
        ADCONFIG()->load(ad);

        STATUS()->display_ad_messages(ad, this);

        console->go_online(ad);
        connect_action->setEnabled(false);
    }
}

void MainWindow::manual() {
    const QString help_collection_path = QStandardPaths::writableLocation(QStandardPaths::QStandardPaths::AppDataLocation) + "/admc.qhc";

    // NOTE: load .qch file from sources for
    // debug/development builds. This is so that you can
    // edit the help file and see changes on the fly without
    // having to install it.
    const QString compressed_help_path =
    []() {
        #ifdef QT_DEBUG
        trace();
        return QCoreApplication::applicationDirPath() + "/doc/admc.qch";
        #endif        

        return QStandardPaths::locate(QStandardPaths::GenericDataLocation, "admc.qch");
    }();

    qDebug() << ".qhc = " << help_collection_path;
    qDebug() << ".qch = " << compressed_help_path;

    auto help_engine = new QHelpEngine(help_collection_path, this);
    const bool help_setup_success = help_engine->setupData();
    if (!help_setup_success) {
        qDebug() << "help_engine setupData() call failed";
        qDebug() << "Help engine error : " << qPrintable(help_engine->error());
    }

    const bool help_register_success = help_engine->registerDocumentation(compressed_help_path);
    if (!help_register_success) {
        qDebug() << "help_engine registerDocumentation() call failed";
        qDebug() << "Help engine error : " << qPrintable(help_engine->error());
    }

    auto tab_widget = new QTabWidget();
    tab_widget->addTab(help_engine->contentWidget(), "Contents");
    tab_widget->addTab(help_engine->indexWidget(), "Index");

    auto help_browser = new HelpBrowser(help_engine);
    
    auto splitter = new QSplitter(Qt::Horizontal);
    splitter->insertWidget(0, tab_widget);
    splitter->insertWidget(1, help_browser);

    auto help_dialog = new QDialog();
    help_dialog->setAttribute(Qt::WA_DeleteOnClose);
    help_dialog->setMinimumSize(800, 600);

    help_dialog->setLayout(new QVBoxLayout());
    help_dialog->layout()->addWidget(splitter);
    
    help_dialog->open();
}

void MainWindow::about() {
    auto dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    auto version_label = new QLabel(QString(tr("Version %1")).arg(ADMC_VERSION));
    version_label->setAlignment(Qt::AlignHCenter);

    auto description_label = new QLabel(tr("ADMC is a tool for Active Directory administration."));

    auto license_label = new QLabel(tr("Copyright (C) 2020 BaseALT Ltd."));

    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);

    auto layout = new QVBoxLayout();
    dialog->setLayout(layout);
    layout->addWidget(version_label);
    layout->addWidget(description_label);
    layout->addWidget(license_label);
    layout->addWidget(button_box);

    connect(
        ok_button, &QPushButton::clicked,
        dialog, &QDialog::accept);

    dialog->open();
}

void MainWindow::open_toggle_widgets_dialog() {
    auto dialog = new ToggleWidgetsDialog(this);
    dialog->open();
}

void MainWindow::quit() {
    QApplication::quit();
}
