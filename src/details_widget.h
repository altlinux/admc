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

#ifndef DETAILS_WIDGET_H
#define DETAILS_WIDGET_H

#include <QWidget>

class QTreeView;
class QString;
class AttributesWidget;
class MembersWidget;
class ObjectContextMenu;
class ContainersWidget;
class ContentsWidget;
class QTabWidget;
class QLabel;

// Shows info about entry's attributes in multiple tabs
// Targeted at a particular entry
// Updates targets of all tabs when target changes
class DetailsWidget final : public QWidget {
Q_OBJECT

public:
    DetailsWidget(ObjectContextMenu *entry_context_menu, ContainersWidget *containers_widget, ContentsWidget *contents_widget, QWidget *parent);

public slots:
    void on_containers_clicked_dn(const QString &dn);
    void on_contents_clicked_dn(const QString &dn);
    void on_context_menu_details(const QString &dn);

private slots:
    void on_logged_in();
    void on_ad_modified();

private:
    QTabWidget *tab_widget = nullptr;
    AttributesWidget *attributes_widget = nullptr;
    MembersWidget *members_widget = nullptr;
    QLabel *title_label = nullptr;
    QString target_dn;

    void change_target(const QString &dn);
};

#endif /* DETAILS_WIDGET_H */
