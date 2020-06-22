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

class AdModel;
class EntryProxyModel;
class EntryModel;
class QString;

// Shows name, category and description of children of entry selected in containers view
class ContentsWidget final : public EntryWidget {
Q_OBJECT

public:
    ContentsWidget(EntryModel *model, QWidget *parent);

public slots:
    void on_containers_clicked_dn(const QString &dn);

private:
    EntryProxyModel *proxy = nullptr;
    EntryModel *model = nullptr;
    
};

#endif /* CONTENTS_WIDGET_H */
