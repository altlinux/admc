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

// Shows names and values of attributes of the entry selected in contents view
class AttributesWidget final : public QTabWidget {
Q_OBJECT

public:
    AttributesWidget();

public slots:
    void change_model_target(const QString &new_target_dn);

private:
    enum Column {
        Name,
        Value,
        COUNT,
    };

    AttributesModel *model = nullptr;
    QTreeView *view = nullptr;
    QWidget *widget = nullptr;
};

#endif /* ATTRIBUTES_WIDGET_H */
