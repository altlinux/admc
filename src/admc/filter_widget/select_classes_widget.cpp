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
#include "ad/adldap.h"
#include "globals.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QDialog>
#include <QCheckBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>

SelectClassesWidget::SelectClassesWidget(const QList<QString> classes)
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

    dialog = new QDialog(this);

    auto select_all_button = new QPushButton(tr("Select all"));
    auto clear_selection_button = new QPushButton(tr("Clear selection"));

    auto dialog_buttons = new QDialogButtonBox();
    ok_button = dialog_buttons->addButton(QDialogButtonBox::Ok);
    dialog_buttons->addButton(QDialogButtonBox::Cancel);

    auto checks_layout = new QFormLayout();
    for (const QString &object_class : classes) {
        auto check = new QCheckBox();
        check->setChecked(true);
        
        dialog_checks[object_class] = check;

        const QString class_display = adconfig->get_class_display_name(object_class);
        checks_layout->addRow(class_display, check);

        connect(
            check, &QCheckBox::stateChanged,
            this, &SelectClassesWidget::on_check_changed);
    }

    auto dialog_layout = new QVBoxLayout();
    dialog->setLayout(dialog_layout);
    dialog_layout->addLayout(checks_layout);
    dialog_layout->addWidget(select_all_button);
    dialog_layout->addWidget(clear_selection_button);
    dialog_layout->addWidget(dialog_buttons);
    
    connect(
        dialog_buttons, &QDialogButtonBox::accepted,
        dialog, &QDialog::accept);
    connect(
        dialog_buttons, &QDialogButtonBox::rejected,
        dialog, &QDialog::reject);
    connect(
        select_all_button, &QPushButton::clicked,
        this, &SelectClassesWidget::select_all);
    connect(
        clear_selection_button, &QPushButton::clicked,
        this, &SelectClassesWidget::clear_selection);
    connect(
        dialog, &QDialog::accepted,
        this, &SelectClassesWidget::on_dialog_accepted);
    on_dialog_accepted();

    connect(
        select_classes_button, &QAbstractButton::clicked,
        dialog, &QDialog::open);
}

QString SelectClassesWidget::get_filter() const {
    const QList<QString> class_filters =
    [this]() {
        QList<QString> out;

        const QList<QString> selected_classes = get_selected_classes();
        
        for (const QString &object_class : selected_classes) {
            const QString class_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, object_class);

            out.append(class_filter);
        }

        return out;
    }();

    return filter_OR(class_filters);
}

// Display selected classes in line edit as a sorted list of
// class display strings separated by ","
// "User, Organizational Unit, ..."
void SelectClassesWidget::on_dialog_accepted() {
    const QString classes_display_text =
    [this]() {
        const QList<QString> selected_classes = get_selected_classes();
        
        QList<QString> classes_display_strings;
        for (const QString &object_class : selected_classes) {
            const QString class_display = adconfig->get_class_display_name(object_class);
            classes_display_strings.append(class_display);
        }

        std::sort(classes_display_strings.begin(), classes_display_strings.end());

        const QString joined = classes_display_strings.join(", ");

        return joined;
    }();

    classes_display->setText(classes_display_text);
    classes_display->setCursorPosition(0);
}

void SelectClassesWidget::select_all() {
    for (QCheckBox *check : dialog_checks.values()) {
        check->setChecked(true);
    }
}

void SelectClassesWidget::clear_selection() {
    for (QCheckBox *check : dialog_checks.values()) {
        check->setChecked(false);
    }
}

void SelectClassesWidget::on_check_changed() {
    const bool any_checked =
    [this]() {
        for (QCheckBox *check : dialog_checks.values()) {
            if (check->isChecked()) {
                return true;
            }
        }

        return false;
    }();

    ok_button->setEnabled(any_checked);
}

QList<QString> SelectClassesWidget::get_selected_classes() const {
    QList<QString> out;

    for (const QString &object_class : dialog_checks.keys()) {
        const QCheckBox *check = dialog_checks[object_class];

        if (check->isChecked()) {
            out.append(object_class);
        }
    }

    return out;
}

