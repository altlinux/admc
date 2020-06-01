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
#if !defined(__GPGUI_MAINWINDOW_H)
#define __GPGUI_MAINWINDOW_H 1

#include <QMainWindow>
#include <QtWidgets>
#include <QTableWidget>
#include <QFileDialog>

#include "REG_DWORD_Dialog.h"

#include "preg_parser.h"

QT_BEGIN_NAMESPACE
class QMenu;
class QFileDialog;
class QTableWidget;
QT_END_NAMESPACE

namespace qgui {

class MainWindow : public QMainWindow {
    // Q_OBJECT
    QMenu *file_menu;
    QMenu *help_menu;

    QFileDialog *preg_open_dialog;
    QFileDialog *preg_save_dialog;

    QTableWidget *regpol_table;

    REG_DWORD_Dialog *reg_dword_dialog;

    void create_menu_bar();
    void create_status_bar();

  public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void edit_reg_dword_dialog();

  private slots:
    void about();
    void open_preg();
    void save_preg();
    void save_dotreg();

  protected:
    void closeEvent(QCloseEvent *event) override;

  private:
    void preg_entry2table(preg::entry &pentry);
}; /* class MainWindow */

} /* namespace qgui */

#endif /* __GPGUI_MAINWINDOW_H */
