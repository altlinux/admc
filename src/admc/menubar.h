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

#ifndef MENUBAR_H
#define MENUBAR_H

#include <QMenuBar>

class MainWindow;
class ConsoleWidget;

class MenuBar final : public QMenuBar {
Q_OBJECT

public:
    // NOTE: main window and console have must have finished
    // creating their actions before MenuBar ctor is called
    MenuBar(MainWindow *main_window, ConsoleWidget *console_widget);

private slots:
    void quit();
    void open_toggle_widgets_dialog();
    void manual();
    void about();

private:

};

#endif /* MENUBAR_H */
