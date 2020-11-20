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

#ifndef UNLOCK_EDIT_H
#define UNLOCK_EDIT_H

// Unlocks the account, if pressed down (when being applied,
// NOT immediately. Doesn't implement the reverse operation
// (locking) because that's only doable by the server.

#include "edits/attribute_edit.h"

class QCheckBox;

class UnlockEdit final : public AttributeEdit {
Q_OBJECT
public:
    UnlockEdit(QList<AttributeEdit *> *edits_out, QObject *parent);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    QCheckBox *check;
};

#endif /* UNLOCK_EDIT_H */
