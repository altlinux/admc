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

#include "filter_dialog.h"

#include "adldap.h"
#include "filter_classes_widget.h"
#include "filter_custom_dialog.h"
#include "settings.h"

#include <QDialogButtonBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

#define FILTER_CUSTOM_DIALOG_STATE "FILTER_CUSTOM_DIALOG_STATE"
#define FILTER_CLASSES_STATE "FILTER_CLASSES_STATE"

// TODO: implement canceling. Need to be able to load/unload
// filter widget state though. For example, one way to
// implement would be to save old state on open, then reload
// it when cancel is pressed.

FilterDialog::FilterDialog(AdConfig *adconfig, QWidget *parent)
: QDialog(parent) {
    setWindowTitle(tr("Edit Console Filter"));
    resize(400, 400);

    const QList<QString> noncontainer_classes = adconfig->get_noncontainer_classes();

    custom_dialog = new FilterCustomDialog(adconfig, this);

    all_button = new QRadioButton(tr("Show all"));
    classes_button = new QRadioButton(tr("Show only these types"));
    custom_button = new QRadioButton(tr("Create custom"));

    all_button->setChecked(true);

    custom_dialog_button = new QPushButton(tr("Custom"));

    filter_classes_widget = new FilterClassesWidget(adconfig, noncontainer_classes);

    auto button_box = new QDialogButtonBox();
    button_box->addButton(QDialogButtonBox::Ok);

    auto classes_row_layout = new QVBoxLayout();
    classes_row_layout->addWidget(classes_button);
    classes_row_layout->addWidget(filter_classes_widget);

    auto custom_row_layout = new QHBoxLayout();
    custom_row_layout->addWidget(custom_button);
    custom_row_layout->addWidget(custom_dialog_button);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(all_button);
    layout->addLayout(classes_row_layout);
    layout->addLayout(custom_row_layout);
    layout->addWidget(button_box);

    settings_setup_dialog_geometry(SETTING_filter_dialog_geometry, this);

    button_state_name_map = {
        {"ALL_BUTTON_STATE", all_button},
        {"CLASSES_BUTTON_STATE", classes_button},
        {"CUSTOM_BUTTON_STATE", custom_button},
    };

    const QHash<QString, QVariant> state = settings_get_variant(SETTING_filter_dialog_state).toHash();
    
    custom_dialog->restore_state(state[FILTER_CUSTOM_DIALOG_STATE]);
    filter_classes_widget->restore_state(state[FILTER_CLASSES_STATE]);

    for (const QString &state_name : button_state_name_map.keys()) {
        QRadioButton *button = button_state_name_map[state_name];

        const QVariant button_state = state[state_name];
        button->setChecked(button_state.toBool());
    }

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        custom_dialog_button, &QPushButton::clicked,
        custom_dialog, &QDialog::open);

    connect(
        custom_button, &QAbstractButton::toggled,
        this, &FilterDialog::on_custom_button);
    on_custom_button();

    connect(
        classes_button, &QAbstractButton::toggled,
        this, &FilterDialog::on_classes_button);
    on_classes_button();
}

bool FilterDialog::filtering_ON() const {
    return !all_button->isChecked();
}

FilterDialog::~FilterDialog() {
    QHash<QString, QVariant> state;

    state[FILTER_CUSTOM_DIALOG_STATE] = custom_dialog->save_state();
    state[FILTER_CLASSES_STATE] = filter_classes_widget->save_state();

    for (const QString &state_name : button_state_name_map.keys()) {
        QRadioButton *button = button_state_name_map[state_name];
   
        state[state_name] = button->isChecked();
    }

    settings_set_variant(SETTING_filter_dialog_state, state);
}

QString FilterDialog::get_filter() const {
    if (all_button->isChecked()) {
        return "(objectClass=*)";
    } else if (classes_button->isChecked()) {
        return filter_classes_widget->get_filter();
    } else if (custom_button->isChecked()) {
        return custom_dialog->get_filter();
    }

    return QString();
}

void FilterDialog::on_custom_button() {
    const bool checked = custom_button->isChecked();

    custom_dialog_button->setEnabled(checked);
}

void FilterDialog::on_classes_button() {
    const bool checked = classes_button->isChecked();

    filter_classes_widget->setEnabled(checked);
}
