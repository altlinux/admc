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

#ifndef BROWSE_WIDGET_H
#define BROWSE_WIDGET_H

#include <QWidget>
#include <QPoint>
#include <QString>

class QTreeView;
class QStandardItemModel;
class QStandardItem;
class QString;

class BrowseWidget final : public QWidget {
Q_OBJECT

public:
    BrowseWidget();

    void change_target(const QString &policy_path_arg);

private slots:
    void on_context_menu(const QPoint pos);

private:
    QStandardItemModel *model = nullptr;
    QTreeView *view = nullptr;
    QString policy_path;

    void add_entry_recursively(const QString &path, QStandardItem *parent);
};

#endif /* BROWSE_WIDGET_H */
