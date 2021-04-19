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

#ifndef MOVE_QUERY_DIALOG_H
#define MOVE_QUERY_DIALOG_H

/**
 * Move query item or folder to a query folder. Shows query
 * folders as a tree.
 */

#include <QDialog>

class QTreeView;
class QStandardItemModel;
class QSortFilterProxyModel;
class ConsoleWidget;

class MoveQueryDialog : public QDialog {
Q_OBJECT

public:
    MoveQueryDialog(ConsoleWidget *console_arg);

    void open() override;
    void accept() override;
    
private:
    QTreeView *view;
    QStandardItemModel *model;
    QSortFilterProxyModel *proxy_model;
    ConsoleWidget *console;
};

#endif /* MOVE_QUERY_DIALOG_H */
