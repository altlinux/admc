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

#ifndef HELP_BROWSER_H
#define HELP_BROWSER_H

/**
 * Slightly modified version of QTextBrowser for displaying
 * qt help files loaded by QHelpEngine. Connects to help
 * engine's content and index widgets to load help files
 * when their links are activated.
 */

#include <QTextBrowser>

class QHelpEngine;

class HelpBrowser : public QTextBrowser {
    Q_OBJECT

public:
    HelpBrowser(QWidget *parent = nullptr);

    void init(QHelpEngine *help_engine_arg);

    QVariant loadResource(int type, const QUrl &name) override;

private:
    QHelpEngine *help_engine;

    void on_content_clicked(const QModelIndex &index);
};

#endif /* HELP_BROWSER_H */
