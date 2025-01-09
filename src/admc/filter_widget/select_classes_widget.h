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

#ifndef SELECT_CLASSES_WIDGET_H
#define SELECT_CLASSES_WIDGET_H

/**
 * Widget embedded in find widget for selecting classes to
 * filter for. Displays currently selected classes in line
 * edit, with a button next to it which opens a dialog in
 * which classes can be selected.
 */

#include <QVariant>
#include <QWidget>

namespace Ui {
class SelectClassesWidget;
}

class SelectClassesWidget final : public QWidget {
    Q_OBJECT

public:
    Ui::SelectClassesWidget *ui;

    SelectClassesWidget(QWidget *parent = nullptr);
    ~SelectClassesWidget();

    void set_classes(const QList<QString> &class_list, const QList<QString> &selected_list);
    void enable_filtering_all_classes();

    QString get_filter() const;

    QVariant save_state() const;
    void restore_state(const QVariant &state);

private:
    QList<QString> class_list;
    QList<QString> m_selected_list;
    bool filtering_all_classes_is_enabled;
    bool m_all_is_checked;

    void open_dialog();
    void update_class_display();
};

#endif /* SELECT_CLASSES_WIDGET_H */
