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

#include "console_filter_dialog.h"
#include "ui_console_filter_dialog.h"

#include "adldap.h"
#include "filter_widget/class_filter_widget.h"
#include "filter_widget/filter_dialog.h"
#include "globals.h"
#include "settings.h"

#define FILTER_CUSTOM_DIALOG_STATE "FILTER_CUSTOM_DIALOG_STATE"
#define FILTER_CLASSES_STATE "FILTER_CLASSES_STATE"

ConsoleFilterDialog::ConsoleFilterDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::ConsoleFilterDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    const QList<QString> class_list_for_widget = {
        CLASS_USER,
        CLASS_GROUP,
        CLASS_CONTACT,
        CLASS_COMPUTER,
        CLASS_PRINTER,
        CLASS_SHARED_FOLDER,
    };
    ui->class_filter_widget->set_classes(class_list_for_widget, class_list_for_widget);

    custom_filter = settings_get_variant(SETTING_object_filter).toString();

    const int object_display_limit = settings_get_variant(SETTING_object_display_limit).toInt();
    ui->limit_spinbox->setValue(object_display_limit);

    settings_setup_dialog_geometry(SETTING_console_filter_dialog_geometry, this);

    button_state_name_map = {
        {"ALL_BUTTON_STATE", ui->all_button},
        {"CLASSES_BUTTON_STATE", ui->classes_button},
        {"CUSTOM_BUTTON_STATE", ui->custom_button},
    };

    connect(
        ui->custom_dialog_button, &QPushButton::clicked,
        this, &ConsoleFilterDialog::open_custom_dialog);

    connect(
        ui->custom_button, &QAbstractButton::toggled,
        this, &ConsoleFilterDialog::on_custom_button);
    on_custom_button();

    connect(
        ui->classes_button, &QAbstractButton::toggled,
        this, &ConsoleFilterDialog::on_classes_button);
    on_classes_button();
}

bool ConsoleFilterDialog::get_filter_enabled() const {
    return !ui->all_button->isChecked();
}

ConsoleFilterDialog::~ConsoleFilterDialog() {
    delete ui;
}

void ConsoleFilterDialog::accept() {
    const int object_display_limit = ui->limit_spinbox->value();
    settings_set_variant(SETTING_object_display_limit, object_display_limit);

    QDialog::accept();
}

QVariant ConsoleFilterDialog::save_state() const {
    QHash<QString, QVariant> state;

    state[FILTER_CUSTOM_DIALOG_STATE] = filter_dialog_state;
    state[FILTER_CLASSES_STATE] = ui->class_filter_widget->save_state();

    for (const QString &state_name : button_state_name_map.keys()) {
        QRadioButton *button = button_state_name_map[state_name];

        state[state_name] = button->isChecked();
    }

    return QVariant(state);
}

void ConsoleFilterDialog::restore_state(const QVariant &state) {
    const QHash<QString, QVariant> state_hash = state.toHash();

    filter_dialog_state = state_hash[FILTER_CUSTOM_DIALOG_STATE];
    ui->class_filter_widget->restore_state(state_hash[FILTER_CLASSES_STATE]);

    for (const QString &state_name : button_state_name_map.keys()) {
        QRadioButton *button = button_state_name_map[state_name];

        const QVariant button_state = state_hash[state_name];
        button->setChecked(button_state.toBool());
    }
}

QString ConsoleFilterDialog::get_filter() const {
    if (ui->all_button->isChecked()) {
        return "(objectClass=*)";
    } else if (ui->classes_button->isChecked()) {
        return ui->class_filter_widget->get_filter();
    } else if (ui->custom_button->isChecked()) {
        return custom_filter;
    }

    return QString();
}

void ConsoleFilterDialog::open_custom_dialog() {
    // NOTE: Using only non-container classes for filtering
    // because container classes need to always be visible
    const QList<QString> noncontainer_classes = g_adconfig->get_noncontainer_classes();

    auto dialog = new FilterDialog(noncontainer_classes, noncontainer_classes, this);

    dialog->restore_state(filter_dialog_state);

    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog]() {
            filter_dialog_state = dialog->save_state();

            custom_filter = dialog->get_filter();
        });
}

void ConsoleFilterDialog::on_custom_button() {
    const bool checked = ui->custom_button->isChecked();

    ui->custom_dialog_button->setEnabled(checked);
}

void ConsoleFilterDialog::on_classes_button() {
    const bool checked = ui->classes_button->isChecked();

    ui->class_filter_widget->setEnabled(checked);
}
