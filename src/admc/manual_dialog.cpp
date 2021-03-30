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

#include "manual_dialog.h"

#include "help_browser.h"

#include <QDebug>
#include <QString>
#include <QHelpEngine>
#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QTabWidget>
#include <QStandardPaths>
#include <QSplitter>
#include <QVBoxLayout>
#include <QCoreApplication>

ManualDialog::ManualDialog(QWidget *parent)
: QDialog(parent)
{
    setMinimumSize(800, 600);

    const QString help_collection_path = QStandardPaths::writableLocation(QStandardPaths::QStandardPaths::AppDataLocation) + "/admc.qhc";

    // NOTE: load .qch file from sources for
    // debug/development builds. This is so that you can
    // edit the help file and see changes on the fly without
    // having to install it.
    const QString compressed_help_path =
    []() {
        #ifdef QT_DEBUG
        return QCoreApplication::applicationDirPath() + "/doc/admc.qch";
        #endif        

        return QStandardPaths::locate(QStandardPaths::GenericDataLocation, "admc.qch");
    }();

    if (compressed_help_path.isEmpty()) {
        qInfo() << "Failed to find manual file. Check that program was installed correctly. (If you are a developer and want to load uninstalled help file from source, set build type to debug.";

        return;
    }

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

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(splitter);
}
