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

#include "entry_widget.h"
#include "ad_interface.h"

#include <QString>

class ContainersModel;
class EntryProxyModel;
class EntryModel;
class QString;
class ContainersWidget;
class QStandardItem;
class EntryContextMenu;

// Shows name, category and description of children of entry selected in containers view
class ContentsWidget final : public EntryWidget {
Q_OBJECT

public:
    ContentsWidget(EntryModel* model_arg, ContainersWidget *containers_widget, EntryContextMenu *entry_context_menu, QWidget *parent);

    void change_target(const QString &dn);

private slots:
    void on_create_entry_complete(const QString &dn, NewEntryType type);
    void on_dn_changed(const QString &old_dn, const QString &new_dn);
    void on_attributes_changed(const QString &dn);

private:
    enum Column {
        Name,
        Category,
        Description,
        DN,
        COUNT,
    };

    EntryModel *model = nullptr;
    EntryProxyModel *proxy = nullptr;
    QString target_dn = "";
    
    void remove_child(const QString &dn);
    void load_row(QList<QStandardItem *> row, const QString &dn);
    void make_new_row(QStandardItem *parent, const QString &dn);

};

#endif /* CONTENTS_WIDGET_H */
