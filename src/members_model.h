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

#include <QStandardItemModel>

class QString;

class MembersModel final : public QStandardItemModel {
Q_OBJECT

public:
    enum Column {
        Name,
        DN,
        COUNT,
    };

    explicit MembersModel(QObject *parent);

    void change_target(const QString &new_target_dn);

    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

private:
    QString target_dn;

    static QString get_dn_of_index(const QModelIndex &index);
    QString get_parent_dn(const QModelIndex &parent) const;

};

#endif /* MEMBERS_MODEL_H */
