/*
 * GPGUI - Group Policy Editor GUI
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
#if !defined(MAIN_WINDOW_H)
#define MAIN_WINDOW_H 1

#include <QMainWindow>
#include <QtWidgets>
#include <QTableWidget>
#include <QFileDialog>

#include "preg_parser.h"

class QMenu;
class QFileDialog;
class QTableWidget;
class BrowseWidget;

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(const QString &path);
    void edit_reg_dword_dialog();

private slots:
    void open_preg();
    void save_preg();
    void save_dotreg();
    void on_exit();

    void on_open_local_dir();
    void on_open_local_xml();
    void on_open_local_pol();
    void on_open_path();

private:
    BrowseWidget *browse_widget;
    QMenu *help_menu;

    QFileDialog *preg_open_dialog;
    QFileDialog *preg_save_dialog;

    QTableWidget *regpol_table;

    void create_menu_bar();
    void create_status_bar();
    void preg_entry2table(preg::entry &pentry);
    void open_generic_path(const QString &path);
};

#endif /* MAIN_WINDOW_H */
