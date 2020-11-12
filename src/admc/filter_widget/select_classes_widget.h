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
#include <QList>

class QLineEdit;

extern const QList<QString> search_classes;

class SelectClassesWidget final : public QWidget {
public:
    SelectClassesWidget();

    // Return a filter that accepts only selected classes
    QString get_filter() const;

private slots:
    void select_classes();

private:
    QLineEdit *classes_display;
    QDialog *select_dialog;

    QList<QString> selected;
};

#endif /* SELECT_CLASSES_WIDGET_H */
