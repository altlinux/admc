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

#include "edits/attribute_edit.h"

#include "tabs/properties_tab.h"
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

    m_modified = false;
    connect(
        this, &AttributeEdit::edited,
        [this]() {
            m_modified = true;
        });
}

void AttributeEdit::load(AdInterface &ad, const AdObject &object) {
    load_internal(ad, object);
    m_modified = false;
}

bool AttributeEdit::verify(AdInterface &ad, const QString &dn) const {
    UNUSED_ARG(ad);
    UNUSED_ARG(dn);

    return true;
}

bool AttributeEdit::modified() const {
    return m_modified;
}

void AttributeEdit::set_modified(const bool modified) {
    m_modified = modified;
}

void AttributeEdit::reset_modified() {
    m_modified = false;
}

bool edits_verify(AdInterface &ad, QList<AttributeEdit *> edits, const QString &dn, const bool ignore_modified) {
    for (auto edit : edits) {
        if (edit->modified() || ignore_modified) {
            const bool verify_success = edit->verify(ad, dn);
            if (!verify_success) {
                return false;
            }
        }
    }

    return true;
}

bool edits_apply(AdInterface &ad, QList<AttributeEdit *> edits, const QString &dn) {
    bool success = true;

    for (auto edit : edits) {
        if (edit->modified()) {
            const bool apply_success = edit->apply(ad, dn);
            if (apply_success) {
                edit->reset_modified();
            } else {
                success = false;
            }
        }
    }

    return success;
}

void edits_load(QList<AttributeEdit *> edits, AdInterface &ad, const AdObject &object) {
    for (auto edit : edits) {
        edit->load(ad, object);
    }
}

void edits_connect_to_tab(QList<AttributeEdit *> edits, PropertiesTab *tab) {
    for (auto edit : edits) {
        QObject::connect(
            edit, &AttributeEdit::edited,
            tab, &PropertiesTab::on_edit_edited);
    }
}

void edits_set_read_only(QList<AttributeEdit *> edits, const bool read_only) {
    for (AttributeEdit *edit : edits) {
        edit->set_read_only(read_only);
    }
}

void edits_set_modified(QList<AttributeEdit *> edits, const bool modified) {
    for (AttributeEdit *edit : edits) {
        edit->set_modified(modified);
    }
}
