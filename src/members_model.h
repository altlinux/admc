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

#ifndef MEMBERS_MODEL_H
#define MEMBERS_MODEL_H

#include "entry_model.h"

class QString;
class QModelIndex;

// Model for MembersWidget
// Contains columns for DN(from EntryModel) and name
class MembersModel final : public EntryModel {
Q_OBJECT

public:
    enum Column {
        Name,
        DN,
        COUNT,
    };

    explicit MembersModel(QObject *parent);

    void change_target(const QString &new_target_dn);

private slots:
    void on_move_complete(const QString &dn, const QString &new_container, const QString &new_dn);

private:
    QString target_dn;

protected:
    QString get_dn_from_index(const QModelIndex &index) const;

};

#endif /* MEMBERS_MODEL_H */
