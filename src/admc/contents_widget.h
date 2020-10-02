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

#ifndef CONTENTS_WIDGET_H
#define CONTENTS_WIDGET_H

#include "ad_interface_defines.h"
#include "object_model.h"

#include <QWidget>
#include <QString>
#include <QList>
#include <QHash>

class ContainersWidget;
class QStandardItem;
class ObjectContextMenu;
class QTreeView;
class ContentsModel;
class QLabel;

// Shows name, category and description of children of object selected in containers view
class ContentsWidget final : public QWidget {
Q_OBJECT

public:
    ContentsWidget(ContainersWidget *containers_widget, QWidget *parent);

private slots:
    void on_containers_selected_changed(const QString &dn);
    void on_ad_modified();
    void on_view_clicked(const QModelIndex &index);

private:
    QString target_dn = "";
    ContentsModel *model = nullptr;
    QTreeView *view = nullptr;
    QLabel *label = nullptr;

    void change_target(const QString &dn);
    void resize_columns();
    void showEvent(QShowEvent *event);

};

class ContentsModel final : public ObjectModel {
Q_OBJECT

public:
    ContentsModel(QObject *parent);

    void change_target(const QString &dn);

private:
    void make_row(QStandardItem *parent, const Attributes &attributes);
};

#endif /* CONTENTS_WIDGET_H */
