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

#include "adldap.h"
#include "globals.h"
#include "filter_classes_widget.h"

#include <QVBoxLayout>
#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>

SelectClassesWidget::SelectClassesWidget(const QList<QString> class_list)
: QWidget()
{
    classes_display = new QLineEdit();
    classes_display->setReadOnly(true);

    auto select_classes_button = new QPushButton(tr("Select"));
    select_classes_button->setAutoDefault(false);

    auto layout = new QHBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(classes_display);
    layout->addWidget(select_classes_button);

    auto dialog = new QDialog(this);

    filter_classes_widget = new FilterClassesWidget(class_list);

    auto dialog_buttons = new QDialogButtonBox();
    dialog_buttons->addButton(QDialogButtonBox::Ok);

    auto dialog_layout = new QVBoxLayout();
    dialog->setLayout(dialog_layout);
    dialog_layout->addWidget(filter_classes_widget);
    dialog_layout->addWidget(dialog_buttons);
    
    connect(
        dialog_buttons, &QDialogButtonBox::accepted,
        dialog, &QDialog::accept);
    connect(
        dialog, &QDialog::accepted,
        this, &SelectClassesWidget::update_classes_display);
    update_classes_display();

    connect(
        select_classes_button, &QAbstractButton::clicked,
        dialog, &QDialog::open);
}

QString SelectClassesWidget::get_filter() const {
    return filter_classes_widget->get_filter();
}

// Display selected classes in line edit as a sorted list of
// class display strings separated by ","
// "User, Organizational Unit, ..."
void SelectClassesWidget::update_classes_display() {
    const QString classes_display_text =
    [this]() {
        const QList<QString> selected_classes = filter_classes_widget->get_selected_classes();
        
        QList<QString> classes_display_strings;
        for (const QString &object_class : selected_classes) {
            const QString class_display = g_adconfig->get_class_display_name(object_class);
            classes_display_strings.append(class_display);
        }

        std::sort(classes_display_strings.begin(), classes_display_strings.end());

        const QString joined = classes_display_strings.join(", ");

        return joined;
    }();

    classes_display->setText(classes_display_text);
    classes_display->setCursorPosition(0);
}

void SelectClassesWidget::save_state(QHash<QString, QVariant> &state) const {
    filter_classes_widget->save_state(state);
}

void SelectClassesWidget::load_state(const QHash<QString, QVariant> &state) {
    filter_classes_widget->load_state(state);
    update_classes_display();
}
