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
#include "ad_interface.h"
#include "settings.h"
#include "toggle_widgets_dialog.h"
#include "status.h"
#include "object_menu.h"
#include "config.h"
#include "help_browser.h"

#include <QMenu>
#include <QLocale>
#include <QMessageBox>
#include <QActionGroup>
#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include <QHelpEngine>
#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QTabWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QStandardPaths>

MenuBar::MenuBar()
: QMenuBar() {
    file_menu = addMenu(tr("&File"));

    connect_action = file_menu->addAction(tr("&Connect"));

    file_menu->addAction(tr("&Quit"),
        []() {
            QApplication::quit();
        });

    action_menu = addMenu(tr("&Action"));

    auto navigation_menu = addMenu(tr("&Navigation"));
    up_one_level_action = navigation_menu->addAction(tr("&Up one level"));
    back_action = navigation_menu->addAction(tr("&Back"));
    forward_action = navigation_menu->addAction(tr("&Forward"));

    auto add_bool_setting_action = 
    [](QMenu *menu, QString display_text, BoolSetting type) {
        QAction *action = menu->addAction(display_text);
        SETTINGS()->connect_action_to_bool_setting(action, type);
    };

    QMenu *view_menu = addMenu(tr("&View"));
    add_bool_setting_action(view_menu, tr("&Advanced view"), BoolSetting_AdvancedView);
    auto toggle_widgets_action = view_menu->addAction(tr("&Toggle widgets"));
    filter_action = view_menu->addAction(tr("&Filter objects"));

    QMenu *preferences_menu = addMenu(tr("&Preferences"));
    add_bool_setting_action(preferences_menu, tr("&Confirm actions"), BoolSetting_ConfirmActions);

    #ifdef QT_DEBUG
    add_bool_setting_action(preferences_menu, tr("Dev mode"), BoolSetting_DevMode);
    #endif
    
    add_bool_setting_action(preferences_menu, tr("&Show non-container objects in Console tree"), BoolSetting_ShowNonContainersInConsoleTree);
    add_bool_setting_action(preferences_menu, tr("&Put last name before first name when creating users"), BoolSetting_LastNameBeforeFirstName);

    QMenu *language_menu = preferences_menu->addMenu(tr("&Language"));
    auto language_group = new QActionGroup(language_menu);

    auto add_language_action =
    [this, language_menu, language_group] (QLocale::Language language) {
        QLocale locale(language);
        const QString language_name =
        [locale]() {
            // NOTE: Russian nativeLanguageName starts with lowercase letter for some reason
            QString out = locale.nativeLanguageName();

            const QChar first_letter_uppercased = out[0].toUpper();

            out.replace(0, 1, first_letter_uppercased);

            return out;
        }();

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
            });
    };

    add_language_action(QLocale::English);
    add_language_action(QLocale::Russian);

    QMenu *help_menu = addMenu(tr("&Help"));
    help_menu->addAction(tr("&Manual"), this, &MenuBar::manual);
    help_menu->addAction(tr("&About ADMC"), this, &MenuBar::about);

    connect(
        toggle_widgets_action, &QAction::triggered,
        [this]() {
            auto dialog = new ToggleWidgetsDialog(this);
            dialog->open();
        });
}

void MenuBar::go_online() {
    file_menu->removeAction(connect_action);
}

void MenuBar::manual() {
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

void MenuBar::about() {
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
