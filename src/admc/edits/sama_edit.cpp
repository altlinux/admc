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

#include "edits/sama_edit.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"

#include <QFormLayout>
#include <QLineEdit>

SamaEdit::SamaEdit(QLineEdit *sama_edit, QLineEdit *domain_edit_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    edit = sama_edit;
    domain_edit = domain_edit_arg;

    init();
}

SamaEdit::SamaEdit(QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    edit = new QLineEdit();
    domain_edit = nullptr;

    init();
}

void SamaEdit::init() {
    limit_edit(edit, ATTRIBUTE_SAMACCOUNT_NAME);

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void SamaEdit::load_internal(AdInterface &ad, const AdObject &object) {
    const QString value = object.get_string(ATTRIBUTE_SAMACCOUNT_NAME);
    edit->setText(value);

    load_domain();
}

void SamaEdit::set_read_only(const bool read_only) {
    edit->setDisabled(read_only);
}

void SamaEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_SAMACCOUNT_NAME, CLASS_USER) + ":";

    domain_edit = new QLineEdit();
    domain_edit->setEnabled(false);
    load_domain();

    auto sublayout = new QHBoxLayout();
    sublayout->addWidget(edit);
    sublayout->addWidget(domain_edit);
    layout->addRow(label_text, sublayout);
}

bool SamaEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = edit->text();
    const bool success = ad.attribute_replace_string(dn, ATTRIBUTE_SAMACCOUNT_NAME, new_value);

    return success;
}

void SamaEdit::load_domain() {
    const QString domain_text = []() {
        const QString domain = g_adconfig->domain();
        const QString domain_name = domain.split(".")[0];
        const QString out = domain_name + "\\";

        return out;
    }();

    domain_edit->setText(domain_text);
}
