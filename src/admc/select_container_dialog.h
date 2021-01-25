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

#ifndef SELECT_CONTAINER_DIALOG_H
#define SELECT_CONTAINER_DIALOG_H

/**
 * Displays a tree of container objects, similarly to
 * Containers widget. User can selected a container.
 */

#include <QDialog>

class QTreeView;
class QModelIndex;
class QStandardItemModel;

class SelectContainerDialog final : public QDialog {
Q_OBJECT

public:
    SelectContainerDialog(QWidget *parent);

    QString get_selected() const;

private:
    QTreeView *view;
    QStandardItemModel *model;

    void fetch_node(const QModelIndex &index);
};

#endif /* SELECT_CONTAINER_DIALOG_H */
