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

#include "entry_widget.h"
#include "ad_interface.h"
#include "entry_model.h"

class QItemSelection;
class AdModel;
class EntryProxyModel;

// Display tree of container entries
// And some other "container-like" entries like domain, built-in, etc
// Only shows the name column
class ContainersWidget final : public EntryWidget {
Q_OBJECT

public:
    ContainersWidget(AdModel *model_arg, QWidget *parent);

signals:
    void selected_changed(const QString &dn);

private slots:
    void on_selection_changed(const QItemSelection &selected, const QItemSelection &);

    void on_ad_interface_login_complete(const QString &search_base, const QString &head_dn);
    void on_attributes_changed(const QString &dn);
    void on_delete_entry_complete(const QString &dn); 
    void on_dn_changed(const QString &old_dn, const QString &new_dn);
    void on_create_entry_complete(const QString &dn, NewEntryType type); 

private:
    AdModel *model = nullptr;
    EntryProxyModel *proxy = nullptr;

};

// Load nodes iteratively, as they are expanded by user
// Unexpanded nodes show the expander even if they have no children
// If the node has no children, the expander goes away
class AdModel final : public EntryModel {

public:
    enum Column {
        Name,
        DN,
        COUNT,
    };

    enum Roles {
        CanFetch = Qt::UserRole + 1,
    };

    explicit AdModel(QObject *parent);

    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);
    bool hasChildren(const QModelIndex &parent) const override;

};

QIcon get_entry_icon(const QString &dn);

#endif /* CONTAINERS_WIDGET_H */
