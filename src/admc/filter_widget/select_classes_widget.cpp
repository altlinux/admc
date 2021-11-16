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
#include "filter_widget/ui_select_classes_widget.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"
#include "filter_widget/class_filter_dialog.h"

SelectClassesWidget::SelectClassesWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::SelectClassesWidget();
    ui->setupUi(this);

    connect(
        ui->select_button, &QAbstractButton::clicked,
        this, &SelectClassesWidget::open_dialog);
}

SelectClassesWidget::~SelectClassesWidget() {
    delete ui;
}

void SelectClassesWidget::set_classes(const QList<QString> &class_list_arg, const QList<QString> &selected_list_arg) {
    class_list = class_list_arg;

    selected_list = selected_list_arg;
    update_class_display();
}

QString SelectClassesWidget::get_filter() const {
    return filter;
}

QVariant SelectClassesWidget::save_state() const {
    QHash<QString, QVariant> state;

    const QList<QVariant> selected_list_variant = string_list_to_variant_list(selected_list);
    state["selected_list"] = selected_list_variant;

    return state;
}

void SelectClassesWidget::restore_state(const QVariant &state_variant) {
    QHash<QString, QVariant> state = state_variant.toHash();

    const QList<QVariant> saved_selected_list_variant = state["selected_list"].toList();
    const QList<QString> saved_selected_list = variant_list_to_string_list(saved_selected_list_variant);

    selected_list = saved_selected_list;
    update_class_display();
}

void SelectClassesWidget::open_dialog() {
    auto dialog = new ClassFilterDialog(this);
    dialog->set_classes(class_list, selected_list);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        [this, dialog]() {
            const QList<QString> new_selected_list = dialog->get_selected_classes();
            selected_list = new_selected_list;
            update_class_display();

            filter = dialog->get_filter();
        });
}

void SelectClassesWidget::update_class_display() {
    // Convert class list to list of class display strings,
    // then sort it and finally join by comma's
    const QString display_string = [&]() {
        QList<QString> class_display_list;

        for (const QString &object_class : selected_list) {
            const QString class_display = g_adconfig->get_class_display_name(object_class);

            class_display_list.append(class_display);
        }

        std::sort(class_display_list.begin(), class_display_list.end());

        const QString joined = class_display_list.join(", ");

        return joined;
    }();

    ui->classes_display->setText(display_string);

    // NOTE: set cursor to start because by default,
    // changing text causes line edit to scroll to the end
    ui->classes_display->setCursorPosition(0);
}
