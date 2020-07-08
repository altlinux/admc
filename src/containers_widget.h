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

#ifndef CONTAINERS_WIDGET_H
#define CONTAINERS_WIDGET_H

#include "ad_interface.h"
#include "entry_model.h"

#include <QWidget>

class QItemSelection;
class AdvancedViewProxy;
class EntryContextMenu;
class QTreeView;
class ContainersModel;

class ContainersWidget final : public QWidget {
Q_OBJECT

public:
    ContainersWidget(EntryContextMenu *entry_context_menu, QWidget *parent);

signals:
    void selected_changed(const QString &dn);
    void clicked_dn(const QString &dn);

private slots:
    void on_selection_changed(const QItemSelection &selected, const QItemSelection &);

private:
    ContainersModel *model = nullptr;
    QTreeView *view = nullptr;
};

class ContainersModel final : public EntryModel {
Q_OBJECT

public:
    enum Roles {
        CanFetch = Qt::UserRole + 1,
    };

    ContainersModel(QObject *parent);

    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);
    bool hasChildren(const QModelIndex &parent) const override;

private slots:
    void on_ad_interface_login_complete(const QString &search_base, const QString &head_dn);
    void on_ad_modified();
};

#endif /* CONTAINERS_WIDGET_H */
