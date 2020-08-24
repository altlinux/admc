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

#ifndef ADDRESS_TAB_H
#define ADDRESS_TAB_H

#include "details_tab.h"
#include <QList>

class DetailsWidget;
class QComboBox;
class AttributeEdit;

// Shows member objects of targeted group
class AddressTab final : public DetailsTab {
Q_OBJECT

public:
    AddressTab(DetailsWidget *details_arg);

    void apply();
    bool accepts_target() const;

private:
    QComboBox *country_combo;
    int original_country_code;
    QList<AttributeEdit *> edits;

    void reload();
};

#endif /* ADDRESS_TAB_H */
