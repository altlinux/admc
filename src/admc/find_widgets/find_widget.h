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

#ifndef FIND_WIDGET_H
#define FIND_WIDGET_H

/**
 * Provides a way for user to find objects. FilterWidget is
 * used for filter input and FindResults for displaying
 * objects. Used by FindObjectDialog and SelectObjectDialog.
 */

#include <QWidget>

class QStandardItem;
class AdObject;
class QMenu;
class ObjectImpl;
class ConsoleWidget;

namespace Ui {
class FindWidget;
}

class FindWidget final : public QWidget {
    Q_OBJECT

public:
    Ui::FindWidget *ui;

    FindWidget(QWidget *parent = nullptr);
    ~FindWidget();

    void set_classes(const QList<QString> &class_list, const QList<QString> &selected_list);
    void set_default_base(const QString &default_base);

    // NOTE: this is only for the console state, filter
    // widget is untouched
    QVariant save_console_state() const;
    void restore_console_state(const QVariant &state);

    void setup_action_menu(QMenu *menu);
    void setup_view_menu(QMenu *menu);

    void set_buddy_console(ConsoleWidget *buddy_console);

    // NOTE: returned items need to be re-parented or deleted!
    QList<QString> get_selected_dns() const;

    void enable_filtering_all_classes();

private slots:
    void find();
    void handle_find_thread_results(const QHash<QString, AdObject> &results);

private:
    ObjectImpl *object_impl;
    QStandardItem *head_item;

    QAction *action_view_icons;
    QAction *action_view_list;
    QAction *action_view_detail;
    QAction *action_customize_columns;
    QAction *action_toggle_description_bar;

    void on_clear_button();
    void clear_results();
};

#endif /* FIND_WIDGET_H */
