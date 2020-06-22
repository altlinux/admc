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

#include <QWidget>
#include <QList>
#include <QSet>

class QTreeView;
class QLabel;
class EntryModel;
class QString;

// Widget based on a QTreeView with an EntryModel
class EntryWidget : public QWidget {
Q_OBJECT

public:
    EntryWidget(EntryModel *model, QWidget *parent);

signals:
    void clicked_dn(const QString &dn);

private slots:
    void on_toggle_show_dn_column(bool checked);
    void on_view_clicked(const QModelIndex &index);
    void on_ad_interface_login_complete(const QString &base, const QString &head);

protected:
    QTreeView *view = nullptr;
    QList<bool> column_hidden;
    
    void update_column_visibility();

private:
    EntryModel *entry_model = nullptr;

};

#endif /* ENTRY_WIDGET_H */
