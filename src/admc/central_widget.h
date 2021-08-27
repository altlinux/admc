/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#ifndef CENTRAL_WIDGET_H
#define CENTRAL_WIDGET_H

/**
 * The central widget of the app through which user can
 * browse and manipulate objects. Wraps ConsoleWidget inside it.
 */

#include <QWidget>

class QAbstractItemView;
class QStandardItemModel;
class QModelIndex;
class QString;
class QMenu;
class QPoint;
class QStandardItem;
class AdObject;
class QLabel;
class QSortFilterProxyModel;
class AdInterface;
class ConsoleWidget;
template <typename T>
class QList;
class QToolBar;

enum ItemType {
    ItemType_Unassigned,
    ItemType_Object,
    ItemType_PolicyRoot,
    ItemType_Policy,
    ItemType_QueryFolder,
    ItemType_QueryItem,

    ItemType_LAST,
};

class CentralWidget final : public QWidget {
    Q_OBJECT

public:
    CentralWidget(AdInterface &ad, QMenu *action_menu);
    ~CentralWidget();

    void add_actions(QMenu *view_menu, QMenu *preferences_menu, QToolBar *toolbar);

signals:
    void context_menu(const QPoint pos);

private slots:
    void on_show_non_containers();
    void on_dev_mode();
    void on_advanced_features();

private:
    ConsoleWidget *console;

    QAction *open_filter_action;
    QAction *show_noncontainers_action;
    QAction *dev_mode_action;
    QAction *advanced_features_action;

    void enable_disable_helper(const bool disabled);
    void refresh_object_tree();
};

#endif /* CENTRAL_WIDGET_H */
