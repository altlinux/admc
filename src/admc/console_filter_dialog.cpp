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

#include "console_filter_dialog.h"
#include "ui_console_filter_dialog.h"

#include "adldap.h"
#include "filter_classes_widget.h"
#include "filter_widget/filter_dialog.h"
#include "settings.h"

#define FILTER_CUSTOM_DIALOG_STATE "FILTER_CUSTOM_DIALOG_STATE"
#define FILTER_CLASSES_STATE "FILTER_CLASSES_STATE"

// TODO: implement canceling. Need to be able to load/unload
// filter widget state though. For example, one way to
// implement would be to save old state on open, then reload
// it when cancel is pressed.

ConsoleFilterDialog::ConsoleFilterDialog(AdConfig *adconfig, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::ConsoleFilterDialog();
    ui->setupUi(this);

    // NOTE: Using only non-container classes for filtering
    // because container classes need to always be visible
    const QList<QString> noncontainer_classes = adconfig->get_noncontainer_classes();

    custom_dialog = new FilterDialog(this);
    custom_dialog->add_classes(adconfig, noncontainer_classes);

    ui->filter_classes_widget->add_classes(adconfig, noncontainer_classes);

    settings_setup_dialog_geometry(SETTING_filter_dialog_geometry, this);

    button_state_name_map = {
        {"ALL_BUTTON_STATE", ui->all_button},
        {"CLASSES_BUTTON_STATE", ui->classes_button},
        {"CUSTOM_BUTTON_STATE", ui->custom_button},
    };

    const QHash<QString, QVariant> settings_state = settings_get_variant(SETTING_filter_dialog_state).toHash();
    restore_state(settings_state);

    connect(
        ui->custom_dialog_button, &QPushButton::clicked,
        custom_dialog, &QDialog::open);

    connect(
        ui->custom_button, &QAbstractButton::toggled,
        this, &ConsoleFilterDialog::on_custom_button);
    on_custom_button();

    connect(
        ui->classes_button, &QAbstractButton::toggled,
        this, &ConsoleFilterDialog::on_classes_button);
    on_classes_button();
}

bool ConsoleFilterDialog::filtering_ON() const {
    return !ui->all_button->isChecked();
}

ConsoleFilterDialog::~ConsoleFilterDialog() {
    const QVariant state = save_state();

    settings_set_variant(SETTING_filter_dialog_state, state);

    delete ui;
}

void ConsoleFilterDialog::open() {
    original_state = save_state();

    QDialog::open();
}

void ConsoleFilterDialog::reject() {
    restore_state(original_state);

    QDialog::reject();
}

QVariant ConsoleFilterDialog::save_state() const {
    QHash<QString, QVariant> state;

    state[FILTER_CUSTOM_DIALOG_STATE] = custom_dialog->save_state();
    state[FILTER_CLASSES_STATE] = ui->filter_classes_widget->save_state();

    for (const QString &state_name : button_state_name_map.keys()) {
        QRadioButton *button = button_state_name_map[state_name];
   
        state[state_name] = button->isChecked();
    }

    return QVariant(state);
}

void ConsoleFilterDialog::restore_state(const QVariant &state) {
    const QHash<QString, QVariant> state_hash = state.toHash();

    custom_dialog->restore_state(state_hash[FILTER_CUSTOM_DIALOG_STATE]);
    ui->filter_classes_widget->restore_state(state_hash[FILTER_CLASSES_STATE]);

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
        return ui->filter_classes_widget->get_filter();
    } else if (ui->custom_button->isChecked()) {
        return custom_dialog->get_filter();
    }

    return QString();
}

void ConsoleFilterDialog::on_custom_button() {
    const bool checked = ui->custom_button->isChecked();

    ui->custom_dialog_button->setEnabled(checked);
}

void ConsoleFilterDialog::on_classes_button() {
    const bool checked = ui->classes_button->isChecked();

    ui->filter_classes_widget->setEnabled(checked);
}
