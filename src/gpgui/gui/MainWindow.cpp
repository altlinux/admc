/*
 * GPGUI - Group Policy Editor GUI
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
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
#include "browse_widget.h"
#include "xml_editor.h"

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
#include <QFileDialog>
#include <QMenu>
#include <QApplication>
#include <QFileDialog>

#include "MainWindow.h"

#include "preg_writer.h"

MainWindow::MainWindow(const QString &path)
: QMainWindow()
{
    // this->preg_open_dialog = new QFileDialog(
    //     this, tr("Select PReg file to edit"), QDir::currentPath(),
    //     "PReg files (*.pol);;All files (*.*)");
    // this->preg_open_dialog->setFileMode(QFileDialog::ExistingFile);

    // this->preg_save_dialog = new QFileDialog(
    //     this, tr("Select PReg file to save"), QDir::currentPath(),
    //     "PReg files (*.pol);;All files (*.*)");
    // this->preg_save_dialog->setFileMode(QFileDialog::AnyFile);

    // QVBoxLayout *layout_regpol_editor = new QVBoxLayout;
    // QVBoxLayout *layout_gpo_editor = new QVBoxLayout;

    // QTabWidget *tw = new QTabWidget;
    // tw->addTab(new QWidget, tr("PReg editor"));
    // tw->addTab(new QWidget, tr("GPO editor"));
    // tw->addTab(new QWidget, tr("Domain Control"));

    // this->regpol_table = new QTableWidget(0, 4, this);
    // QStringList labels{"Value name", "Key name", "Type", "Value"};
    // this->regpol_table->setHorizontalHeaderLabels(labels);
    // this->regpol_table->horizontalHeader()->setStretchLastSection(true);
    // tw->widget(0)->setLayout(layout_regpol_editor);
    // layout_regpol_editor->addWidget(regpol_table);

    // QVBoxLayout *layout = new QVBoxLayout;
    // layout->addWidget(tw);

    // QFrame *frame = new QFrame;
    // frame->setLayout(layout);
    // frame->setFixedSize(800, 500);
    // setCentralWidget(frame);
    // this->adjustSize();

    // /* Create dialog windows */
    // this->reg_dword_dialog = new REG_DWORD_Dialog();

    const auto menubar = new QMenuBar();
    setMenuBar(menubar);

    auto file_menu = menubar->addMenu(tr("File"));

    auto open_local_dir = file_menu->addAction(tr("Open local GPO directory"));
    connect(
        open_local_dir, &QAction::triggered,
        this, &MainWindow::on_open_local_dir);

    auto open_local_xml = file_menu->addAction(tr("Open local xml file"));
    connect(
        open_local_xml, &QAction::triggered,
        this, &MainWindow::on_open_local_xml);

    auto open_local_pol = file_menu->addAction(tr("Open local pol file"));
    connect(
        open_local_pol, &QAction::triggered,
        this, &MainWindow::on_open_local_pol);
    open_local_pol->setEnabled(false);

    auto open_path = file_menu->addAction(tr("Open path"));
    connect(
        open_path, &QAction::triggered,
        this, &MainWindow::on_open_path);

    auto exit_action = file_menu->addAction(tr("Exit"));
    connect(
        exit_action, &QAction::triggered,
        this, &MainWindow::on_exit);

    browse_widget = new BrowseWidget();
    
    const auto central_layout = new QVBoxLayout();
    central_layout->addWidget(browse_widget);

    const auto central_widget = new QWidget();
    setCentralWidget(central_widget);
    central_widget->setLayout(central_layout);

    // NOTE: default dir for testing
    if (path.isEmpty()) {
        browse_widget->change_target("/home/kevl/pol-files/{2BE174FB-22F4-4CD1-BA93-5311F87E80A2}");
    }

    XmlEditor::load_schema();

    open_generic_path(path);
}

void MainWindow::preg_entry2table(preg::entry &pentry) {
    static const QStringList regtype_list = {
        "REG_NONE",
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
        "REG_QWORD_LITTLE_ENDIAN"
    };

    std::string val = "0"; //std::to_string(pentry.value);

    QComboBox *regtype_box = new QComboBox();
    regtype_box->addItems(regtype_list);
    regtype_box->setEditable(false);
    regtype_box->setMaxVisibleItems(6);
    regtype_box->setCurrentIndex(static_cast<int>(pentry.type));

    QTableWidgetItem *vname = new QTableWidgetItem(pentry.value_name.c_str());
    QTableWidgetItem *kname = new QTableWidgetItem(pentry.key_name.c_str());
    QTableWidgetItem *vval = new QTableWidgetItem(val.c_str());

    this->regpol_table->insertRow(regpol_table->rowCount());

    this->regpol_table->setItem(this->regpol_table->rowCount() - 1, 0, vname);
    this->regpol_table->setItem(this->regpol_table->rowCount() - 1, 1, kname);
    this->regpol_table->setCellWidget(this->regpol_table->rowCount() - 1, 2, regtype_box);
    this->regpol_table->setItem(this->regpol_table->rowCount() - 1, 3, vval);
}

void MainWindow::open_preg() {
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

void MainWindow::save_preg() {
    QStringList preg_file_name;
    /* Using exec() is not recommended by Qt documentation but it
     * is easy to perform synchronous call */
    if (this->preg_save_dialog->exec()) {
        preg_file_name = this->preg_save_dialog->selectedFiles();

        std::string file_name = preg_file_name[0].toStdString();

        preg::preg_writer pw(file_name);
        for (int rowid = 0; rowid < this->regpol_table->rowCount(); rowid++) {
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

void MainWindow::save_dotreg() {}

void MainWindow::edit_reg_dword_dialog() {
}

void MainWindow::on_exit() {
    QApplication::closeAllWindows();
    QApplication::quit();
}

void MainWindow::on_open_local_dir() {
    const QString path = QFileDialog::getExistingDirectory(this, tr("Open directory"), "home/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    open_generic_path(path);
}

void MainWindow::on_open_local_xml() {
    const QString path = QFileDialog::getOpenFileName(this, tr("Open XML file"), "/home", tr("XML files (*.xml)"));
    open_generic_path(path);
}

void MainWindow::on_open_local_pol() {
    const QString path = QFileDialog::getOpenFileName(this, tr("Open POL file"), "/home", tr("POL files (*.pol)"));
    open_generic_path(path);
}

void MainWindow::on_open_path() {
    bool ok;
    const QString path = QInputDialog::getText(this, tr("Open path"), tr("Path:"), QLineEdit::Normal, QString(), &ok);

    if (ok && !path.isEmpty()) {
        open_generic_path(path);
    }
}

void MainWindow::open_generic_path(const QString &path) {
    if (path.isEmpty()) {
        return;
    } else if (path.endsWith(".xml")) {
        auto xml_editor = new XmlEditor(path);
        xml_editor->open();
    } else if (path.endsWith(".xml")) {
        // auto pol_editor = new PolEditor(path);
        // pol_editor->open();
    } else {
        // TODO: this could be a non xml/pol file
        browse_widget->change_target(path);
    }
}

void MainWindow::closeEvent(QCloseEvent *) {

}
