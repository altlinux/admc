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

#ifndef GPLINK_INVERSE_TAB_H
#define GPLINK_INVERSE_TAB_H

#include "details_tab.h"

#include <QHash>
#include <QPoint>
#include <QSet>
#include <QString>

class QTreeView;
class QString;
class ObjectContextMenu;
class MembersModel;
class QStandardItemModel;

class GplinkInverseTab final : public DetailsTab {
Q_OBJECT

public:
    GplinkInverseTab(DetailsWidget *details_arg);
    DECL_DETAILS_TAB_VIRTUALS();

private slots:
    void on_context_menu(const QPoint pos);

private:
    QStandardItemModel *model = nullptr;
    QTreeView *view = nullptr;
};

#endif /* GPLINK_INVERSE_TAB_H */
