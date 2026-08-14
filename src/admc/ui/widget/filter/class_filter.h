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

#ifndef CLASS_FILTER_WIDGET_H
#define CLASS_FILTER_WIDGET_H

/**
 * This widget is embedded in ConsoleFilterDialog. Contains
 * checkboxes of object classes. get_filter() returns a
 * filter which will filter out unselected classes.
 */

#include <QWidget>

class QCheckBox;

namespace Ui {
class ClassFilterWidget;
}

class ClassFilterWidget final : public QWidget {
    Q_OBJECT

public:
    Ui::ClassFilterWidget *ui;

    ClassFilterWidget(QWidget *parent = nullptr);
    ~ClassFilterWidget();

    void set_classes(const QList<QString> &class_list, const QList<QString> &selected_list);

    QString get_filter() const;
    QList<QString> get_selected_classes() const;
    QVariant save_state() const;
    void restore_state(const QVariant &state);

signals:
    // Emitted when any of the checkboxes are touched
    void changed();

private:
    QHash<QString, QCheckBox *> checkbox_map;
    QList<QString> class_list;

    void clear_selection();
    void select_all();
};

#endif /* CLASS_FILTER_WIDGET_H */
