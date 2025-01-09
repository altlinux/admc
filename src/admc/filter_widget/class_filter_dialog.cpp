/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

// NOTE: "all" checkbox functionality is messy due to
// reuse and composition. It is what it is.

ClassFilterDialog::ClassFilterDialog(const QList<QString> &class_list, const QList<QString> &selected_list, const bool filtering_all_classes_is_enabled, const bool all_is_checked, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::ClassFilterDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    ui->class_filter_widget->set_classes(class_list, selected_list);

    original_state = ui->class_filter_widget->save_state();

    ui->all_checkbox->setVisible(filtering_all_classes_is_enabled);
    if (filtering_all_classes_is_enabled) {
        ui->all_checkbox->setChecked(all_is_checked);
    }

    connect(
        ui->button_box->button(QDialogButtonBox::Reset), &QPushButton::clicked,
        this, &ClassFilterDialog::reset);
    connect(
        ui->all_checkbox, &QCheckBox::toggled,
        this, &ClassFilterDialog::on_all_checkbox);
    connect(
        ui->all_checkbox, &QCheckBox::toggled,
        this, &ClassFilterDialog::on_input_changed);
    connect(
        ui->class_filter_widget, &ClassFilterWidget::changed,
        this, &ClassFilterDialog::on_input_changed);

    on_all_checkbox();
    on_input_changed();

    settings_setup_dialog_geometry(SETTING_class_filter_dialog_geometry, this);
}

ClassFilterDialog::~ClassFilterDialog() {
    delete ui;
}

QString ClassFilterDialog::get_filter() const {
    return ui->class_filter_widget->get_filter();
}

QList<QString> ClassFilterDialog::get_selected_classes() const {
    return ui->class_filter_widget->get_selected_classes();
}

bool ClassFilterDialog::get_all_is_checked() const {
    return ui->all_checkbox->isChecked();
}

void ClassFilterDialog::reset() {
    ui->all_checkbox->setChecked(false);
    ui->class_filter_widget->restore_state(original_state);
}

void ClassFilterDialog::on_input_changed() {
    const bool input_is_valid = [&]() {
        const bool all_is_selected = ui->all_checkbox->isChecked();
        const QList<QString> selected_classes = get_selected_classes();
        const bool any_specific_class_selected = !selected_classes.isEmpty();
        const bool out = (all_is_selected || any_specific_class_selected);

        return out;
    }();

    QPushButton *ok_button = ui->button_box->button(QDialogButtonBox::Ok);
    ok_button->setEnabled(input_is_valid);
}

// Disable checkboxes for specific classes when "All"
// checkbox is checked.
void ClassFilterDialog::on_all_checkbox() {
    const bool all_is_checked = ui->all_checkbox->isChecked();
    ui->class_filter_widget->setDisabled(all_is_checked);
}
