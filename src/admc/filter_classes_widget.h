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

#ifndef FILTER_CLASSES_WIDGET_H
#define FILTER_CLASSES_WIDGET_H

/**
 * This widget is embedded in FilterDialog. Contains
 * checkboxes of object classes. get_filter() returns a
 * filter which will filter out unselected classes.
 */

#include <QWidget>
#include <QHash>
#include <QString>

class QCheckBox;

class FilterClassesWidget final : public QWidget {
Q_OBJECT
    
public:
    FilterClassesWidget(const QList<QString> &class_list);

    QString get_filter() const;
    QList<QString> get_selected_classes() const;
    void serialize(QDataStream &stream) const;
    void deserialize(QDataStream &stream);

private:
    QHash<QString, QCheckBox *> checkbox_map;
    QList<QString> class_list;
};

QDataStream &operator<<(QDataStream &stream, const FilterClassesWidget *widget);
QDataStream &operator>>(QDataStream &stream, FilterClassesWidget *widget);

#endif /* FILTER_CLASSES_WIDGET_H */
