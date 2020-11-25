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

#include "filter_widget/select_classes_widget.h"
#include "ad_defines.h"
#include "ad_config.h"
#include "filter.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QDialog>
#include <QCheckBox>
#include <QLineEdit>
#include <QDialogButtonBox>

SelectClassesWidget::SelectClassesWidget()
: QWidget()
{
    classes_display = new QLineEdit();
    classes_display->setReadOnly(true);
    classes_display->setPlaceholderText(tr("All classes"));

    auto select_classes_button = new QPushButton(tr("Select"));
    select_classes_button->setAutoDefault(false);

    auto layout = new QHBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    
    layout->addWidget(classes_display);
    layout->addWidget(select_classes_button);

    connect(
        select_classes_button, &QAbstractButton::clicked,
        this, &SelectClassesWidget::select_classes);
}

QString SelectClassesWidget::get_filter() const {
    const QList<QString> class_filters =
    [this]() {
        QList<QString> out;

        for (const QString object_class : selected) {
            const QString class_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, object_class);

            out.append(class_filter);
        }

        return out;
    }();

    return filter_OR(class_filters);
}

void SelectClassesWidget::select_classes() {
    auto dialog = new QDialog(this);

    auto layout = new QVBoxLayout();
    dialog->setLayout(layout);

    QHash<QString, QCheckBox *> checkboxes;

    auto checkboxes_layout = new QFormLayout();

    for (const QString object_class : filter_classes) {
        auto checkbox = new QCheckBox();
        checkboxes[object_class] = checkbox;

        const bool class_is_selected = selected.contains(object_class);
        checkbox->setChecked(class_is_selected);

        const QString class_display = ADCONFIG()->get_class_display_name(object_class);
        checkboxes_layout->addRow(class_display, checkbox);
    }

    auto button_box = new QDialogButtonBox();
    layout->addLayout(checkboxes_layout);
    layout->addWidget(button_box);
    
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);
    connect(
        ok_button, &QPushButton::clicked,
        dialog, &QDialog::accept);
    connect(
        cancel_button, &QPushButton::clicked,
        dialog, &QDialog::reject);

    connect(
        dialog, &QDialog::accepted,
        [this, checkboxes]() {
            selected.clear();

            // Save selected classes
            for (const QString object_class : filter_classes) {
                QCheckBox *checkbox = checkboxes[object_class];

                if (checkbox->isChecked()) {
                    selected.append(object_class);
                }
            }

            // Display selected classes set as a sorted list
            // of class display strings separated by ","
            // {"user", "organizationUnit"}
            // =>
            // "User, Organizational Unit"
            const QString selected_classes_string =
            [this]() {
                QList<QString> classes_display_strings;
                for (const QString object_class : selected) {
                    const QString class_display = ADCONFIG()->get_class_display_name(object_class);
                    classes_display_strings.append(class_display);
                }

                std::sort(classes_display_strings.begin(), classes_display_strings.end());

                const QString joined = classes_display_strings.join(", ");

                return joined;
            }();
            classes_display->setText(selected_classes_string);
        });

    dialog->open();
}
