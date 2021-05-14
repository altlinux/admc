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

FilterClassesWidget::FilterClassesWidget()
: QWidget()
{   
    // = all classes - container classes
    const QList<QString> noncontainer_classes =
    []() {
        QList<QString> out = filter_classes;

        const QList<QString> container_classes = g_adconfig->get_filter_containers();
        for (const QString &container_class : container_classes) {
            out.removeAll(container_class);
        }

        return out;
    }();

    for (const QString &object_class : noncontainer_classes) {
        const QString class_string = g_adconfig->get_class_display_name(object_class);
        auto checkbox = new QCheckBox(class_string);

        checkbox_map[object_class] = checkbox;
    }

    auto classes_widget = new QWidget();

    auto classes_layout = new QVBoxLayout();
    classes_widget->setLayout(classes_layout);

    for (const QString &object_class : noncontainer_classes) {
        QCheckBox *checkbox = checkbox_map[object_class];
        classes_layout->addWidget(checkbox);
    }

    auto scroll_area = new QScrollArea();
    scroll_area->setWidget(classes_widget);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(scroll_area);
}
