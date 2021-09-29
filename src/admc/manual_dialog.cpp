/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "manual_dialog.h"
#include "ui_manual_dialog.h"

#include "help_browser.h"
#include "settings.h"

#include <QCoreApplication>
#include <QDebug>
#include <QHelpContentWidget>
#include <QHelpEngine>
#include <QHelpIndexWidget>
#include <QSplitter>
#include <QStandardPaths>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QDir>

ManualDialog::ManualDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::ManualDialog();
    ui->setupUi(this);

    const QString help_collection_path = QStandardPaths::writableLocation(QStandardPaths::QStandardPaths::AppDataLocation) + "/admc.qhc";

    // NOTE: load .qch file from sources for
    // debug/development builds. This is so that you can
    // edit the help file and see changes on the fly without
    // having to install it.
    const QString compressed_help_path = []() {
#ifdef QT_DEBUG
        return QCoreApplication::applicationDirPath() + "/doc/admc.qch";
#endif

        return QStandardPaths::locate(QStandardPaths::GenericDataLocation, "doc/admc/admc.qch");
    }();

    if (compressed_help_path.isEmpty()) {
        qInfo() << "Failed to find manual file. Check that program was installed correctly. (If you are a developer and want to load uninstalled help file from source, set build type to debug.";

        return;
    }

    qDebug() << ".qhc = " << help_collection_path;
    qDebug() << ".qch = " << compressed_help_path;

    const QFileInfo help_collection_info = QFileInfo(help_collection_path);
    const QDir help_collection_dir = help_collection_info.absoluteDir();
    if (!help_collection_dir.exists()) {
        help_collection_dir.mkpath(help_collection_dir.absolutePath());
    }

    auto help_engine = new QHelpEngine(help_collection_path, this);
    const bool help_setup_success = help_engine->setupData();
    if (!help_setup_success) {
        qDebug() << "help_engine setupData() call failed";
        qDebug() << "Help engine error : " << qPrintable(help_engine->error());
    }

    help_engine->unregisterDocumentation("alt.basealt.admc");
    const bool help_register_success = help_engine->registerDocumentation(compressed_help_path);
    if (!help_register_success) {
        qDebug() << "help_engine registerDocumentation() call failed";
        qDebug() << "Help engine error : " << qPrintable(help_engine->error());
    }

    auto replace_tab = [=](const int index, QWidget *widget) {
        const QString label = ui->tab_widget->tabText(index);
        ui->tab_widget->removeTab(index);
        ui->tab_widget->insertTab(index, widget, label);
    };

    replace_tab(0, help_engine->contentWidget());
    replace_tab(1, help_engine->indexWidget());

    ui->help_browser->init(help_engine);

    settings_setup_dialog_geometry(SETTING_manual_dialog_geometry, this);
}
