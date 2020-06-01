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
class MembersModel;

// Shows info about entry's attributes in multiple tabs
class AttributesWidget final : public QTabWidget {
Q_OBJECT

public:
    AttributesWidget();

public slots:
    void change_target(const QString &dn);

private:
    AttributesModel *attributes_model = nullptr;
    QTreeView *attributes_view = nullptr;
    
    QTreeView *members_view = nullptr;
    MembersModel *members_model = nullptr;
};

#endif /* ATTRIBUTES_WIDGET_H */
