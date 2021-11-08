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

#include "class_filter_widget.h"
#include "ui_class_filter_widget.h"

#include "adldap.h"

#include <QCheckBox>

ClassFilterWidget::ClassFilterWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::ClassFilterWidget();
    ui->setupUi(this);

    connect(
        ui->select_all_button, &QPushButton::clicked,
        this, &ClassFilterWidget::select_all);
    connect(
        ui->clear_selection_button, &QPushButton::clicked,
        this, &ClassFilterWidget::clear_selection);
}

ClassFilterWidget::~ClassFilterWidget() {
    delete ui;
}

void ClassFilterWidget::init(AdConfig *adconfig, const QList<QString> &class_list, const QList<QString> &selected_list) {
    for (const QString &object_class : class_list) {
        const QString class_string = adconfig->get_class_display_name(object_class);
        auto checkbox = new QCheckBox(class_string);

        checkbox_map[object_class] = checkbox;

        const bool is_selected = selected_list.contains(object_class);
        checkbox->setChecked(is_selected);

        ui->classes_layout->addWidget(checkbox);
    }
}

QString ClassFilterWidget::get_filter() const {
    const QList<QString> class_filter_list = [&] {
        QList<QString> out;

        for (const QString &object_class : checkbox_map.keys()) {
            QCheckBox *checkbox = checkbox_map[object_class];

            if (checkbox->isChecked()) {
                const QString class_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, object_class);
                out.append(class_filter);
            }
        }

        return out;
    }();

    const QString filter = filter_OR(class_filter_list);

    return filter;
}

QList<QString> ClassFilterWidget::get_selected_classes() const {
    QList<QString> out;

    for (const QString &object_class : checkbox_map.keys()) {
        const QCheckBox *check = checkbox_map[object_class];

        if (check->isChecked()) {
            out.append(object_class);
        }
    }

    return out;
}

void ClassFilterWidget::select_all() {
    for (QCheckBox *checkbox : checkbox_map.values()) {
        checkbox->setChecked(true);
    }
}

void ClassFilterWidget::clear_selection() {
    for (QCheckBox *checkbox : checkbox_map.values()) {
        checkbox->setChecked(false);
    }
}

QVariant ClassFilterWidget::save_state() const {
    QHash<QString, QVariant> state;

    for (const QString &object_class : checkbox_map.keys()) {
        QCheckBox *checkbox = checkbox_map[object_class];
        const bool checked = checkbox->isChecked();

        state[object_class] = QVariant(checked);
    }

    return QVariant(state);
}

void ClassFilterWidget::restore_state(const QVariant &state_variant) {
    const QHash<QString, QVariant> state = state_variant.toHash();

    for (const QString &object_class : checkbox_map.keys()) {
        const bool checked = [&]() {
            if (state.contains(object_class)) {
                const bool out = state[object_class].toBool();

                return out;
            } else {
                return false;
            }
        }();

        QCheckBox *checkbox = checkbox_map[object_class];
        checkbox->setChecked(checked);
    }
}
