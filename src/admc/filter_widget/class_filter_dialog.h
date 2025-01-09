/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#ifndef CLASS_FILTER_DIALOG_H
#define CLASS_FILTER_DIALOG_H

/**
 * Dialog to select classes, opened from
 * SelectClassesWidget. Wraps ClassFilterWidget inside.
 */

#include <QDialog>
#include <QVariant>

class ClassFilterWidget;

namespace Ui {
class ClassFilterDialog;
}

class ClassFilterDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::ClassFilterDialog *ui;

    ClassFilterDialog(const QList<QString> &class_list, const QList<QString> &selected_list, const bool filtering_all_classes_is_enabled, const bool all_is_checked, QWidget *parent);
    ~ClassFilterDialog();

    QString get_filter() const;
    QList<QString> get_selected_classes() const;
    bool get_all_is_checked() const;

private:
    QVariant original_state;

    void reset();

    void on_input_changed();
    void on_all_checkbox();
};

#endif /* CLASS_FILTER_DIALOG_H */
