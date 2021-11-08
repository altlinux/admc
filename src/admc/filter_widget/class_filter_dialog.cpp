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

#include "filter_widget/class_filter_dialog.h"
#include "filter_widget/ui_class_filter_dialog.h"

#include "settings.h"

#include <QPushButton>

ClassFilterDialog::ClassFilterDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::ClassFilterDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    connect(
        ui->button_box->button(QDialogButtonBox::Reset), &QPushButton::clicked,
        this, &ClassFilterDialog::reset);

    settings_setup_dialog_geometry(SETTING_class_filter_dialog_geometry, this);
}

ClassFilterDialog::~ClassFilterDialog() {
    delete ui;
}

void ClassFilterDialog::init(AdConfig *adconfig, const QList<QString> &class_list, const QList<QString> &selected_list) {
    return ui->class_filter_widget->init(adconfig, class_list, selected_list);
}

QString ClassFilterDialog::get_filter() const {
    return ui->class_filter_widget->get_filter();
}

QList<QString> ClassFilterDialog::get_selected_classes() const {
    return ui->class_filter_widget->get_selected_classes();
}

QVariant ClassFilterDialog::save_state() const {
    return ui->class_filter_widget->save_state();
}

void ClassFilterDialog::restore_state(const QVariant &state) {
    ui->class_filter_widget->restore_state(state);
}


void ClassFilterDialog::open() {
    // Save state to later restore if dialog is is
    // rejected
    state_to_restore = ui->class_filter_widget->save_state();

    QDialog::open();
}

void ClassFilterDialog::reject() {
    reset();

    QDialog::reject();
}

void ClassFilterDialog::reset() {
    ui->class_filter_widget->restore_state(state_to_restore);
}
