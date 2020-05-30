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

#ifndef ATTRIBUTES_MODEL_H
#define ATTRIBUTES_MODEL_H

#include <QStandardItemModel>

class AttributesModel final : public QStandardItemModel {
Q_OBJECT

public:
    enum Column {
        Name,
        Value,
        COUNT,
    };

    explicit AttributesModel(QObject *parent);

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    void change_target(const QString &new_target_dn);

private slots:
    void on_delete_entry_complete(const QString &dn); 
    void on_move_user_complete(const QString &user_dn, const QString &container_dn, const QString &new_dn); 
    void on_load_attributes_complete(const QString &dn);

private:
    QString target_dn;

};

#endif /* ATTRIBUTES_MODEL_H */
