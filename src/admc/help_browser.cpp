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

#include "help_browser.h"

#include "utils.h"

#include <QHelpContentWidget>
#include <QHelpEngine>
#include <QHelpIndexWidget>
#include <QHelpContentModel>

HelpBrowser::HelpBrowser(QWidget *parent)
: QTextBrowser(parent) {
    
}

void HelpBrowser::init(QHelpEngine *help_engine_arg) {
    help_engine = help_engine_arg;

    // NOTE: use currentChanged() from selection model
    // instead of linkActivated() signal so that can use
    // single-click instead of double-click and also switch
    // pages by keyboard navigation
    QHelpContentWidget *content_widget = help_engine->contentWidget();
    QItemSelectionModel *selection_model = content_widget->selectionModel();
    connect(
        selection_model, &QItemSelectionModel::currentChanged,
        this, &HelpBrowser::on_content_clicked);
    
    connect(
        help_engine->indexWidget(), &QHelpIndexWidget::linkActivated,
        this, QOverload<const QUrl &>::of(&HelpBrowser::setSource));
}

// Need to override this f-n to load file data from help
// engine. Returning file data from this function causes it
// to be loaded by text browser.
QVariant HelpBrowser::loadResource(int type, const QUrl &name) {
    UNUSED_ARG(type);

    const QByteArray file_data = help_engine->fileData(name);

    return QVariant(file_data);
}

void HelpBrowser::on_content_clicked(const QModelIndex &index) {
    const QHelpContentWidget *content_widget = help_engine->contentWidget();
    if (content_widget == nullptr) {
        return;
    }

    const QHelpContentModel *contentModel = qobject_cast<QHelpContentModel*>(content_widget->model());
    if (contentModel == nullptr) {
        return;
    }

    const QHelpContentItem *item = contentModel->contentItemAt(index);
    if (item == nullptr) {
        return;
    }

    const QUrl url = item->url();
    if (url.isValid()) {
        setSource(url);
    }
}
