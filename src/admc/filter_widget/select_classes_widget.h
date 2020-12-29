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

#ifndef SELECT_CLASSES_WIDGET_H
#define SELECT_CLASSES_WIDGET_H

/**
 * Select classes for filtering.
 */

#include <QWidget>
#include <QString>
#include <QHash>

class QLineEdit;
class QDialog;
class QCheckBox;
class QPushButton;

class SelectClassesWidget final : public QWidget {
Q_OBJECT
    
public:
    SelectClassesWidget(const QList<QString> classes);

    // Return a filter that accepts only selected classes
    QString get_filter() const;

private slots:
    void on_dialog_accepted();
    void select_all();
    void clear_selection();
    void on_check_changed();

private:
    QLineEdit *classes_display;
    QDialog *dialog;
    QHash<QString, QCheckBox *> dialog_checks;
    QPushButton *ok_button;

    QList<QString> get_selected_classes() const;
};

#endif /* SELECT_CLASSES_WIDGET_H */
