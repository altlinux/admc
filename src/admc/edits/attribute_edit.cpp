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

#include "edits/attribute_edit.h"
#include "tabs/details_tab.h"

AttributeEdit::AttributeEdit(QList<AttributeEdit *> *edits_out, QObject *parent) {
    if (edits_out != nullptr) {
        if (edits_out->contains(this)) {
            printf("ERROR: attribute edit added twice to list!");
        } else {
            edits_out->append(this);
        }
    }

    m_modified = false;
    connect(
        this, &AttributeEdit::edited,
        [this]() {
            m_modified = true;
        });
}

void AttributeEdit::load(const AdObject &object) {
    load_internal(object);
    m_modified = false;
}

bool AttributeEdit::verify(const QString &dn) const {
    return true;
}

bool AttributeEdit::modified() const {
    return m_modified;
}

void edits_add_to_layout(QList<AttributeEdit *> edits, QFormLayout *layout) {
    for (auto edit : edits) {
        edit->add_to_layout(layout);
    }
}

bool edits_verify(QList<AttributeEdit *> edits, const QString &dn) {
    for (auto edit : edits) {
        if (edit->modified()) {
            const bool verify_success = edit->verify(dn);
            if (!verify_success) {
                return false;
            }
        }
    }

    return true;
}

bool edits_apply(QList<AttributeEdit *> edits, const QString &dn) {
    bool success = true;

    for (auto edit : edits) {
        if (edit->modified()) {
            const bool apply_success = edit->apply(dn);
            if (!apply_success) {
                success = false;
            }
        }
    }

    return success;
}

void edits_load(QList<AttributeEdit *> edits, const AdObject &object) {
    for (auto edit : edits) {
        edit->load(object);
    }
}

void edits_connect_to_tab(QList<AttributeEdit *> edits, DetailsTab *tab) {
    for (auto edit : edits) {
        QObject::connect(
            edit, &AttributeEdit::edited,
            tab, &DetailsTab::on_edit_edited);
    }
}
