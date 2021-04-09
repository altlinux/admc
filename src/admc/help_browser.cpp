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

#include "help_browser.h"

#include <QHelpEngine>
#include <QHelpContentWidget>
#include <QHelpIndexWidget>

HelpBrowser::HelpBrowser(QHelpEngine *help_engine_arg) {
    help_engine = help_engine_arg;

    // Change source when links are activate in help
    // engine's content and index widgets. Calling
    // setSource() causes a call to loadResource()
    connect(
        help_engine->contentWidget(), &QHelpContentWidget::linkActivated,
        this, QOverload<const QUrl &>::of(&HelpBrowser::setSource));
    connect(
        help_engine->indexWidget(), &QHelpIndexWidget::linkActivated,
        this, QOverload<const QUrl &>::of(&HelpBrowser::setSource));
}

// Need to override this f-n to load file data from help
// engine. Returning file data from this function causes it
// to be loaded by text browser.
QVariant HelpBrowser::loadResource(int type, const QUrl &name) {
    const QByteArray file_data = help_engine->fileData(name);

    return QVariant(file_data);
}
