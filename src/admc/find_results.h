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

#ifndef FIND_RESULTS_H
#define FIND_RESULTS_H

/**
 * Used by find dialog to display find results as a list of
 * objects.
 */

#include <QWidget>

class QTreeView;
class QLabel;
class QStandardItemModel;
class QStandardItem;
class QMenu;
class AdObject;
class CustomizeColumnsDialog;
class ResultsView;
class ObjectActions;
template <typename T> class QList;
template <typename K, typename T> class QHash;

class FindResults final : public QWidget {
Q_OBJECT

public:
    FindResults();

    void clear();

    // Append results to list and re-sort
    void load(const QHash<QString, AdObject> &search_results);

    // NOTE: returned items need to be re-parented or deleted!
    QList<QList<QStandardItem *>> get_selected_rows() const;

    void add_actions_to_action_menu(QMenu *menu);
    void add_actions_to_view_menu(QMenu *menu);

signals:
    void context_menu(const QPoint pos);

private slots:
    void properties();
    void delete_objects();
    void rename();
    void move();
    void add_to_group();
    void enable();
    void disable();
    void reset_password();
    void create_user();
    void create_computer();
    void create_ou();
    void create_group();

    void customize_columns();
    void on_context_menu(const QPoint pos);

private:
    ResultsView *view;
    QStandardItemModel *model;
    QLabel *object_count_label;
    QAction *customize_columns_action;
    bool context_menu_enabled;

    ObjectActions *object_actions;
    QAction *properties_action;

    void enable_disable_helper(const bool disabled);
    void update_actions_visibility();
    void create_helper(const QString &object_class);
    QHash<QString, QPersistentModelIndex> get_selected_dns_and_indexes();
    QList<QString> get_selected_dns();
};

#endif /* FIND_RESULTS_H */
