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

#ifndef GENERAL_TAB_H
#define GENERAL_TAB_H

#include "details_tab.h"

#include <QList>

class QLabel;
class DetailsWidget;
class AttributeEdit;
class QStackedLayout;

enum GeneralTabType {
    GeneralTabType_Default,
    GeneralTabType_User,
    GeneralTabType_OU,
    GeneralTabType_Computer,
    GeneralTabType_Group,
    GeneralTabType_Container,
    GeneralTabType_COUNT
};

class GeneralTab final : public DetailsTab {
Q_OBJECT

public:
    GeneralTab(DetailsWidget *details_arg);
    DECL_DETAILS_TAB_VIRTUALS();

private:
    GeneralTabType type;
    QLabel *name_label;
    QList<AttributeEdit *> edits_for_type[GeneralTabType_COUNT];
    QStackedLayout *types_stack;
};

#endif /* GENERAL_TAB_H */
