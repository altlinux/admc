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
#include <QModelIndex>

class MenuBar;
class ContentsWidget;
class ContainersWidget;

class MainWindow final : public QMainWindow {
Q_OBJECT

public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_connected();
    void on_containers_current_changed(const QModelIndex &source_index);
    void navigate_up();
    void navigate_back();
    void navigate_forward();

private:
    MenuBar *menubar;
    ContentsWidget *contents_widget;
    ContainersWidget *containers_widget;

    // Last is closest to current
    QList<QModelIndex> targets_past;
    // First is closest to current
    QList<QModelIndex> targets_future;
    QModelIndex targets_current;

    void update_navigation_actions();
};

#endif /* MAIN_WINDOW_H */
