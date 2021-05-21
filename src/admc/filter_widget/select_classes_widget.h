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
 * Widget embedded in find widget for selecting classes to
 * filter for. Displays currently selected classes in line
 * edit, with a button next to it which opens a dialog in
 * which classes can be selected.
 */

#include <QWidget>

class QLineEdit;
class FilterClassesWidget;

class SelectClassesWidget final : public QWidget {
Q_OBJECT
    
public:
    SelectClassesWidget(const QList<QString> class_list);

    QString get_filter() const;

    void save_state(QHash<QString, QVariant> &state) const;
    void load_state(const QHash<QString, QVariant> &state);

private:
    QLineEdit *classes_display;
    FilterClassesWidget *filter_classes_widget;

    void update_classes_display();
};

#endif /* SELECT_CLASSES_WIDGET_H */
