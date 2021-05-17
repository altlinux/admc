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

#ifndef FILTER_DIALOG_H
#define FILTER_DIALOG_H

/**
 * Dialog used to enter a filter that is applied to objects
 * in the console. Allows showing all objects, objects of
 * certain type or entering a custom filter.
 */

#include <QDialog>

class FilterWidget;
class FilterCustomDialog;
class QRadioButton;
class QPushButton;
class FilterClassesWidget;

class FilterDialog final : public QDialog {
Q_OBJECT

public:
    FilterDialog(QWidget *parent);

    QString get_filter() const;
    bool filtering_ON() const;

private:
    FilterCustomDialog *custom_dialog;
    QRadioButton *all_button;
    QRadioButton *custom_button;
    QRadioButton *classes_button;
    QPushButton *custom_dialog_button;
    FilterClassesWidget *filter_classes_widget;
    FilterWidget *filter_widget;

    void on_custom_button();
    void on_classes_button();
};

#endif /* FILTER_DIALOG_H */
