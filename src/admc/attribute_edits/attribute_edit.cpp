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

#include "attribute_edits/attribute_edit.h"

#include "utils.h"

bool AttributeEdit::verify(AdInterface &ad, const QString &dn) const {
    UNUSED_ARG(ad);
    UNUSED_ARG(dn);

    return true;
}

bool AttributeEdit::verify(const QList<AttributeEdit *> &edit_list, AdInterface &ad, const QString &dn) {
    for (auto edit : edit_list) {
        const bool verify_success = edit->verify(ad, dn);

        if (!verify_success) {
            return false;
        }
    }

    return true;
}

bool AttributeEdit::apply(const QList<AttributeEdit *> &edit_list, AdInterface &ad, const QString &dn) {
    bool success = true;

    for (auto edit : edit_list) {
        const bool apply_success = edit->apply(ad, dn);

        if (!apply_success) {
            success = false;
        }
    }

    return success;
}

void AttributeEdit::load(const QList<AttributeEdit *> &edit_list, AdInterface &ad, const AdObject &object) {
    for (auto edit : edit_list) {
        edit->load(ad, object);
    }
}

void AttributeEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);
    UNUSED_ARG(object);
}

bool AttributeEdit::apply(AdInterface &ad, const QString &dn) const {
    UNUSED_ARG(ad);
    UNUSED_ARG(dn);

    return true;
}

void AttributeEdit::set_enabled(const bool enabled) {
    UNUSED_ARG(enabled);
}
