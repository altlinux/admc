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

#ifndef ATTRIBUTES_WIDGET_H
#define ATTRIBUTES_WIDGET_H

#include <QTabWidget>

class QTreeView;
class QString;
class AttributesModel;
class MembersWidget;

// Shows info about entry's attributes in multiple tabs
// Targeted at a particular entry
// Updates targets of all tabs when target changes
class DetailsWidget final : public QTabWidget {
Q_OBJECT

public:
    DetailsWidget(MembersWidget *members_widget_);

public slots:
    void on_containers_clicked_dn(const QString &dn);
    void on_contents_clicked_dn(const QString &dn);
    void on_context_menu_details(const QString &dn);

private slots:
    void on_ad_interface_login_complete(const QString &search_base, const QString &head_dn);
    void on_delete_entry_complete(const QString &dn); 
    void on_move_complete(const QString &dn, const QString &new_container, const QString &new_dn); 
    void on_attributes_changed(const QString &dn);
    void on_rename_complete(const QString &dn, const QString &new_name, const QString &new_dn);

private:
    AttributesModel *attributes_model = nullptr;
    QTreeView *attributes_view = nullptr;
    MembersWidget *members_widget = nullptr;
    QString target_dn;

    void change_target(const QString &dn);
};

#endif /* ATTRIBUTES_WIDGET_H */
