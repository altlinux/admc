/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#ifndef ITEM_TYPE_H
#define ITEM_TYPE_H

enum ItemType {
    ItemType_Unassigned,
    ItemType_Object,
    ItemType_PolicyRoot,
    ItemType_PolicyOU,
    ItemType_AllPoliciesFolder,
    ItemType_Policy,
    ItemType_QueryFolder,
    ItemType_QueryItem,
    ItemType_FindObject,
    ItemType_FindPolicy,
    ItemType_FoundPolicy,
    ItemType_DomainInfo,

    ItemType_LAST,
};

#endif /* ITEM_TYPE_H */
