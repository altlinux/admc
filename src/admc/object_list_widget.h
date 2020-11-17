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

#ifndef OBJECTS_LIST_WIDGET_H
#define OBJECTS_LIST_WIDGET_H

/**
 * Displays a detailed list of objects with many optional
 * toggleable columns. Can load children of a parent or load
 * all objects matching a filter. Does NOT update on AD
   modifications. Intended to be used as a sub-widget
 * in some other widget.
 */

#include "ad_object.h"

#include <QWidget>
#include <QHash>
#include <QList>
#include <QPoint>

class QTreeView;
class QLabel;
class ObjectModel;

class ObjectListWidget final : public QWidget {
Q_OBJECT

public:
    ObjectListWidget();

    void load_children(const QString &new_parent_dn);
    void load_filter(const QString &filter, const QString &search_base);

private slots:
    void on_context_menu(const QPoint pos);

private:
    QString parent_dn;
    QList<QString> columns;
    QLabel *label;
    ObjectModel *model;
    QTreeView *view;

    void load(const QHash<QString, AdObject> &objects);
    void resize_columns();
    void showEvent(QShowEvent *event);
    int column_index(const QString &attribute);
};

#endif /* OBJECTS_LIST_WIDGET_H */
