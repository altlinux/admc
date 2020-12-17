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

#ifndef OBJECT_MODEL_H
#define OBJECT_MODEL_H

#include <QStandardItemModel>
#include <QList>

class QMimeData;
class QModelIndex;
class QString;
class QStandardItem;
class AdObject;

/**
 * Model used by containers and contents widgets. Contains
 * attributes of objects. Doesn't load the whole directory
 * but instead loads it gradually as user expands objects.
 */

class ObjectModel : public QStandardItemModel {
Q_OBJECT

public:
    enum Roles {
        CanFetch = Qt::UserRole + 1,
        IsContainer = Qt::UserRole + 2,
        AdvancedViewOnly = Qt::UserRole + 3,
    };

    ObjectModel(QObject *parent);

    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);
    bool hasChildren(const QModelIndex &parent) const override;

    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

public slots:
    void on_filter_changed(const QString &filter);

private slots:
    void on_object_added(const QString &dn);
    void on_object_deleted(const QString &dn);
    void on_object_changed(const QString &dn);

private:
    QString current_filter;

    QStandardItem *find_object(const QString &dn) const;
    void reset();
};

void load_attributes_row(const QList<QStandardItem *> row, const AdObject &object);
QList<QString> object_model_header_labels();

#endif /* OBJECT_MODEL_H */
