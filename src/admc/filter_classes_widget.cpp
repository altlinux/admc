/*
 * ADMC - AD Management Center
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

#include "filter_classes_widget.h"

#include "adldap.h"
#include "globals.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>

FilterClassesWidget::FilterClassesWidget(const QList<QString> &class_list)
: QWidget()
{   
    for (const QString &object_class : class_list) {
        const QString class_string = g_adconfig->get_class_display_name(object_class);
        auto checkbox = new QCheckBox(class_string);
        checkbox->setChecked(true);

        checkbox_map[object_class] = checkbox;
    }

    auto classes_widget = new QWidget();

    auto classes_layout = new QVBoxLayout();
    classes_widget->setLayout(classes_layout);

    for (const QString &object_class : class_list) {
        QCheckBox *checkbox = checkbox_map[object_class];
        classes_layout->addWidget(checkbox);
    }

    auto buttonbox = new QDialogButtonBox();
    auto select_all_button = buttonbox->addButton(tr("Select all"), QDialogButtonBox::ActionRole);
    auto clear_selection_button = buttonbox->addButton(tr("Clear selection"), QDialogButtonBox::ActionRole);

    auto scroll_area = new QScrollArea();
    scroll_area->setWidget(classes_widget);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(buttonbox);
    layout->addWidget(scroll_area);

    connect(
        select_all_button, &QPushButton::clicked,
        this, &FilterClassesWidget::select_all);
    connect(
        clear_selection_button, &QPushButton::clicked,
        this, &FilterClassesWidget::clear_selection);
}

QString FilterClassesWidget::get_filter() const {
    const QList<QString> class_filter_list =
    [&] {
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

QList<QString> FilterClassesWidget::get_selected_classes() const {
    QList<QString> out;

    for (const QString &object_class : checkbox_map.keys()) {
        const QCheckBox *check = checkbox_map[object_class];

        if (check->isChecked()) {
            out.append(object_class);
        }
    }

    return out;
}

void FilterClassesWidget::select_all() {
    for (QCheckBox *checkbox : checkbox_map.values()) {
        checkbox->setChecked(true);
    }
}

void FilterClassesWidget::clear_selection() {
    for (QCheckBox *checkbox : checkbox_map.values()) {
        checkbox->setChecked(false);
    }
}
