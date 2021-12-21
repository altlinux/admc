/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include <QDebug>

AttributeEdit::AttributeEdit(QList<AttributeEdit *> *edits_out, QObject *parent)
: QObject(parent) {
    if (edits_out != nullptr) {
        if (edits_out->contains(this)) {
            qDebug() << "ERROR: attribute edit added twice to list!";
        } else {
            edits_out->append(this);
        }
    }
}

bool AttributeEdit::verify(AdInterface &ad, const QString &dn) const {
    UNUSED_ARG(ad);
    UNUSED_ARG(dn);

    return true;
}

bool edits_verify(AdInterface &ad, QList<AttributeEdit *> edits, const QString &dn) {
    for (auto edit : edits) {
        const bool verify_success = edit->verify(ad, dn);

        if (!verify_success) {
            return false;
        }
    }

    return true;
}

bool edits_apply(AdInterface &ad, QList<AttributeEdit *> edits, const QString &dn) {
    bool success = true;

    for (auto edit : edits) {
        const bool apply_success = edit->apply(ad, dn);

        if (!apply_success) {
            success = false;
        }
    }

    return success;
}

void edits_load(QList<AttributeEdit *> edits, AdInterface &ad, const AdObject &object) {
    for (auto edit : edits) {
        edit->load(ad, object);
    }
}

void edits_set_read_only(QList<AttributeEdit *> edits, const bool read_only) {
    for (AttributeEdit *edit : edits) {
        edit->set_read_only(read_only);
    }
}
