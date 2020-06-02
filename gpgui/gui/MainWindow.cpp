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
#include "config.h"

#include <QAction>
#include <QCloseEvent>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtWidgets>

#include <QComboBox>
#include <QHeaderView>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <QDir>
#include <QFileDialog>

#include "MainWindow.h"

#include "preg_writer.h"

namespace {

QStringList regtype_list{"REG_NONE",
                         "REG_SZ",
                         "REG_EXPAND_SZ",
                         "REG_BINARY",
                         "REG_DWORD_LITTLE_ENDIAN",
                         "REG_DWORD_BIG_ENDIAN",
                         "REG_LINK",
                         "REG_MULTI_SZ",
                         "REG_RESOURCE_LIST",
                         "REG_FULL_RESOURCE_DESCRIPTOR",
                         "REG_RESOURCE_REQUIREMENTS_LIST",
                         "REG_QWORD",
                         "REG_QWORD_LITTLE_ENDIAN"};

} /* namespace */

void qgui::MainWindow::create_menu_bar() {
    file_menu = menuBar()->addMenu(tr("&File"));
    help_menu = menuBar()->addMenu(tr("&Help"));

    QAction *open_preg_action = file_menu->addAction(
        tr("&Open PReg file"), this, &qgui::MainWindow::open_preg);
    open_preg_action->setStatusTip(tr("Open PReg file for editing"));
    QAction *save_preg_action = file_menu->addAction(
        tr("&Save PReg file"), this, &qgui::MainWindow::save_preg);
    save_preg_action->setStatusTip(tr("Save active PReg file"));
    QAction *save_reg_action = file_menu->addAction(
        tr("&Save REG file"), this, &qgui::MainWindow::save_dotreg);
    save_reg_action->setStatusTip(tr("Save active PReg file as REG"));
    QAction *exit_action =
        file_menu->addAction(tr("&Exit"), this, &QWidget::close);
    exit_action->setStatusTip(tr("Exit GPGUI"));

    QAction *about_action =
        help_menu->addAction(tr("&About"), this, &qgui::MainWindow::about);
    about_action->setStatusTip(tr("About GPGUI"));
}

void qgui::MainWindow::create_status_bar() {
    this->statusBar()->showMessage(tr("Ready"));
}

qgui::MainWindow::MainWindow(QWidget *parent)
//: QMainWindow(parent)
{
    this->preg_open_dialog = new QFileDialog(
        this, tr("Select PReg file to edit"), QDir::currentPath(),
        "PReg files (*.pol);;All files (*.*)");
    this->preg_open_dialog->setFileMode(QFileDialog::ExistingFile);

    this->preg_save_dialog = new QFileDialog(
        this, tr("Select PReg file to save"), QDir::currentPath(),
        "PReg files (*.pol);;All files (*.*)");
    this->preg_save_dialog->setFileMode(QFileDialog::AnyFile);

    QVBoxLayout *layout_regpol_editor = new QVBoxLayout;
    QVBoxLayout *layout_gpo_editor = new QVBoxLayout;

    QTabWidget *tw = new QTabWidget;
    tw->addTab(new QWidget, tr("PReg editor"));
    tw->addTab(new QWidget, tr("GPO editor"));
    tw->addTab(new QWidget, tr("Domain Control"));

    this->regpol_table = new QTableWidget(0, 4, this);
    QStringList labels{"Value name", "Key name", "Type", "Value"};
    this->regpol_table->setHorizontalHeaderLabels(labels);
    this->regpol_table->horizontalHeader()->setStretchLastSection(true);
    tw->widget(0)->setLayout(layout_regpol_editor);
    layout_regpol_editor->addWidget(regpol_table);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tw);

    QFrame *frame = new QFrame;
    frame->setLayout(layout);
    frame->setFixedSize(800, 500);
    setCentralWidget(frame);
    this->adjustSize();

    this->create_menu_bar();
    this->create_status_bar();

    /* Create dialog windows */
    this->reg_dword_dialog = new qgui::REG_DWORD_Dialog();
}

qgui::MainWindow::~MainWindow() {}

void qgui::MainWindow::closeEvent(QCloseEvent *event) { event->accept(); }

void qgui::MainWindow::about() {
    QMessageBox::about(this, tr("About GPGUI"), tr("GPGUI about"));
}

void qgui::MainWindow::preg_entry2table(preg::entry &pentry) {
    std::string regtype = std::string(preg::regtype2str(pentry.type));
    std::string val = "0"; //std::to_string(pentry.value);

    QComboBox *regtype_box = new QComboBox();
    regtype_box->addItems(regtype_list);
    regtype_box->setEditable(false);
    regtype_box->setMaxVisibleItems(6);
    regtype_box->setCurrentIndex(static_cast<int>(pentry.type));

    QTableWidgetItem *vname = new QTableWidgetItem(pentry.value_name.c_str());
    QTableWidgetItem *kname = new QTableWidgetItem(pentry.key_name.c_str());
    QTableWidgetItem *vtype = new QTableWidgetItem(regtype.c_str());
    QTableWidgetItem *vval = new QTableWidgetItem(val.c_str());

    this->regpol_table->insertRow(regpol_table->rowCount());

    this->regpol_table->setItem(this->regpol_table->rowCount() - 1, 0, vname);
    this->regpol_table->setItem(this->regpol_table->rowCount() - 1, 1, kname);
    this->regpol_table->setCellWidget(this->regpol_table->rowCount() - 1, 2, regtype_box);
    this->regpol_table->setItem(this->regpol_table->rowCount() - 1, 3, vval);
}

void qgui::MainWindow::open_preg() {
    QStringList preg_file_name;
    /* Using exec() is not recommended by Qt documentation but it
     * is easy to perform synchronous call */
    if (this->preg_open_dialog->exec()) {
        preg_file_name = this->preg_open_dialog->selectedFiles();

        std::string file_name = preg_file_name[0].toStdString();
        preg::preg_parser *test_regpol =
            new preg::preg_parser(file_name);
        this->regpol_table->setRowCount(0);

        try {
            while (1) {
                preg::entry pentry = test_regpol->get_next_entry();
                this->preg_entry2table(pentry);
            }
        }
        catch (...) {
            std::cout << "Caught exception" << std::endl;
        }

        this->statusBar()->showMessage(tr("Loaded PReg file"));

        this->regpol_table->horizontalHeader()->sectionResizeMode(QHeaderView::Stretch);
        this->regpol_table->resizeColumnsToContents();
    }
}

void qgui::MainWindow::save_preg() {
    QStringList preg_file_name;
    /* Using exec() is not recommended by Qt documentation but it
     * is easy to perform synchronous call */
    if (this->preg_save_dialog->exec()) {
        preg_file_name = this->preg_save_dialog->selectedFiles();

        std::string file_name = preg_file_name[0].toStdString();

        preg::preg_writer pw(file_name);
        for (size_t rowid = 0; rowid < this->regpol_table->rowCount(); rowid++) {
            QTableWidgetItem *qvname = this->regpol_table->item(rowid, 0);
            QTableWidgetItem *qkname = this->regpol_table->item(rowid, 1);
            QComboBox *qtype  = qobject_cast<QComboBox*>(this->regpol_table->cellWidget(rowid, 2));
            QTableWidgetItem *qval   = this->regpol_table->item(rowid, 3);

            preg::entry pe;
            pe.value_name = const_cast<char*>(qvname->data(Qt::DisplayRole).toString().toStdString().c_str());
            pe.key_name   = const_cast<char*>(qkname->data(Qt::DisplayRole).toString().toStdString().c_str());
            pe.type       = qtype->currentIndex();
            pe.size       = 4;
            pe.value      = const_cast<char*>(qval->data(Qt::DisplayRole).toString().toStdString().c_str());

            pw.add_entry(pe);
        }
    }
}

void qgui::MainWindow::save_dotreg() {}

void qgui::MainWindow::edit_reg_dword_dialog() {
}

