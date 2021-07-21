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

#include "filter_widget/select_classes_widget.h"
#include "filter_widget/select_classes_widget_p.h"

#include "adldap.h"
#include "filter_classes_widget.h"
#include "globals.h"

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

SelectClassesWidget::SelectClassesWidget(const QList<QString> class_list)
: QWidget() {
    dialog = new SelectClassesDialog(class_list, this);
    
    classes_display = new QLineEdit();
    classes_display->setReadOnly(true);

    auto select_classes_button = new QPushButton(tr("Select..."));
    select_classes_button->setAutoDefault(false);

    auto layout = new QHBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(classes_display);
    layout->addWidget(select_classes_button);

    connect(
        select_classes_button, &QAbstractButton::clicked,
        dialog, &QDialog::open);
    connect(
        dialog, &QDialog::finished,
        this, &SelectClassesWidget::update_classes_display);
    update_classes_display();
}

QString SelectClassesWidget::get_filter() const {
    return dialog->filter_classes_widget->get_filter();
}

// Display selected classes in line edit as a sorted list of
// class display strings separated by ","
// "User, Organizational Unit, ..."
void SelectClassesWidget::update_classes_display() {
    const QString classes_display_text = [this]() {
        const QList<QString> selected_classes = dialog->filter_classes_widget->get_selected_classes();

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

QVariant SelectClassesWidget::save_state() const {
    return dialog->filter_classes_widget->save_state();
}

void SelectClassesWidget::restore_state(const QVariant &state) {
    dialog->filter_classes_widget->restore_state(state);
    update_classes_display();
}

SelectClassesDialog::SelectClassesDialog(const QList<QString> class_list, QWidget *parent)
: QDialog(parent) {
    filter_classes_widget = new FilterClassesWidget(class_list);

    auto button_box = new QDialogButtonBox();
    button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);
    auto reset_button = button_box->addButton(QDialogButtonBox::Reset);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(filter_classes_widget);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
    connect(
        reset_button, &QPushButton::clicked,
        this, &SelectClassesDialog::reset);
}

void SelectClassesDialog::open() {
    // Save state to later restore if dialog is dialog is
    // rejected
    state_to_restore = filter_classes_widget->save_state();

    QDialog::open();
}

void SelectClassesDialog::reject() {
    reset();

    QDialog::reject();
}

void SelectClassesDialog::reset() {
    filter_classes_widget->restore_state(state_to_restore);
}
