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

#ifndef AD_MODEL_H
#define AD_MODEL_H

#include "ad_interface.h"
#include "entry_model.h"

// Model for containers and contents widgets
// Contains columns for DN(from EntryModel) and some entry attributes
// For now only has name, category and description columns
// Loads entries only when they become visible in views that use
// this model
class AdModel final : public EntryModel {
Q_OBJECT

public:
    enum Column {
        Name,
        Category,
        Description,
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

private slots:
    void on_ad_interface_login_complete(const QString &search_base, const QString &head_dn);
    void on_attributes_changed(const QString &dn);
    void on_dn_changed(const QString &old_dn, const QString &new_dn);
    void on_create_entry_complete(const QString &dn, NewEntryType type); 

private:
    void set_headers();
    QStandardItem *find_first_row_item(const QString &dn);

};

#endif /* AD_MODEL_H */
