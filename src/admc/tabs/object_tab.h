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

#ifndef OBJECT_TAB_H
#define OBJECT_TAB_H

#include "tabs/details_tab.h"

class AttributeEdit;

// Shows member objects of targeted group
class ObjectTab final : public DetailsTab {
Q_OBJECT

public:
    ObjectTab(const AdObject &object);
    DECL_DETAILS_TAB_VIRTUALS();

private:
    QList<AttributeEdit *> edits;
};

#endif /* OBJECT_TAB_H */
