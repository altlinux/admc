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
#include <QList>
#include <QString>

class QTreeView;
class QString;
class ObjectContextMenu;
class ContainersWidget;
class ContentsWidget;
class QTabWidget;
class QLabel;
class DetailsTab;

// Shows info about object's attributes in multiple tabs
// Targeted at a particular object
// Updates targets of all tabs when target changes
class DetailsWidget final : public QWidget {
Q_OBJECT

public:
    DetailsWidget(ObjectContextMenu *object_context_menu, ContainersWidget *containers_widget, ContentsWidget *contents_widget, QWidget *parent);

    QString get_target() const;

public slots:
    void on_containers_clicked_dn(const QString &dn);
    void on_contents_clicked_dn(const QString &dn);
    void on_context_menu_details(const QString &dn);

private slots:
    void on_logged_in();
    void on_ad_modified();

private:
    QTabWidget *tab_widget = nullptr;
    QLabel *title_label = nullptr;
    QList<DetailsTab *> tabs;
    QString target;

    void reload(const QString &new_target);
};

#endif /* DETAILS_WIDGET_H */
