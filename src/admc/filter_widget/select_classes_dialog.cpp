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

#include "filter_widget/select_classes_dialog.h"
#include "filter_widget/ui_select_classes_dialog.h"

#include <QPushButton>

SelectClassesDialog::SelectClassesDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::SelectClassesDialog();
    ui->setupUi(this);

    connect(
        ui->button_box->button(QDialogButtonBox::Reset), &QPushButton::clicked,
        this, &SelectClassesDialog::reset);
}

SelectClassesDialog::~SelectClassesDialog() {
    delete ui;
}

void SelectClassesDialog::init(AdConfig *adconfig) {
    return ui->filter_classes_widget->init(adconfig);
}

void SelectClassesDialog::set_classes(const QList<QString> &class_list, const QList<QString> &selected_list) {
    return ui->filter_classes_widget->set_classes(class_list, selected_list);
}

QString SelectClassesDialog::get_filter() const {
    return ui->filter_classes_widget->get_filter();
}

QList<QString> SelectClassesDialog::get_selected_classes_display() const {
    return ui->filter_classes_widget->get_selected_classes_display();
}

QVariant SelectClassesDialog::save_state() const {
    return ui->filter_classes_widget->save_state();
}

void SelectClassesDialog::restore_state(const QVariant &state) {
    ui->filter_classes_widget->restore_state(state);
}


void SelectClassesDialog::open() {
    // Save state to later restore if dialog is is
    // rejected
    state_to_restore = ui->filter_classes_widget->save_state();

    QDialog::open();
}

void SelectClassesDialog::reject() {
    reset();

    QDialog::reject();
}

void SelectClassesDialog::reset() {
    ui->filter_classes_widget->restore_state(state_to_restore);
}
