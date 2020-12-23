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

#include <QWidget>
#include <QString>

class ObjectModel;
class QTreeView;
class QModelIndex;
class AdvancedViewProxy;
class QLabel;

/**
 * Shows a list of objects, which are children of a target
 * parent object. Parent object is equal to most recent
 * selection in containers widget. Updates on AD modifications.
 */

class ContentsWidget final : public QWidget {
Q_OBJECT

public:
    QTreeView *view;

    ContentsWidget(ObjectModel *model_arg);

public slots:
    void set_target(const QModelIndex &source_index);

private slots:
    void on_header_toggled();

private:
    QString target_dn;
    ObjectModel *model;
    AdvancedViewProxy *advanced_view_proxy;
    QWidget *header;
    QLabel *header_label;

    void showEvent(QShowEvent *event);
};

#endif /* CONTENTS_WIDGET_H */
