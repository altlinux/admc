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

#ifndef ENTRY_WIDGET_H
#define ENTRY_WIDGET_H

#include "ad_model.h"

#include <QWidget>
#include <QMap>

class QTreeView;
class QAction;
class QLabel;
class AdModel;
class AdProxyModel;

// Shows names of AdModel as a tree
class EntryWidget : public QWidget {
Q_OBJECT

public:
    EntryWidget(AdModel *model);

    QString get_selected_dn() const;

private slots:
    void on_action_toggle_dn(bool checked);
    void on_context_menu_requested(const QPoint &pos);

protected:
    QTreeView *view = nullptr;
    AdProxyModel *proxy = nullptr;
    QLabel *label = nullptr;
    QMap<AdModel::Column, bool> column_hidden;
    
    void update_column_visibility();

};

#endif /* ENTRY_WIDGET_H */
