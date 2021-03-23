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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

class QAction;
class Console;
class QDockWidget;

class MainWindow final : public QMainWindow {
Q_OBJECT

public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private:
    QAction *connect_action;
    Console *console;
    QDockWidget *message_log_dock;

    void setup_menubar();
    void connect_to_server();
    void quit();
};

#endif /* MAIN_WINDOW_H */
