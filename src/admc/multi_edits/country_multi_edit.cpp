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

#include "multi_edits/country_multi_edit.h"

#include "adldap.h"
#include "edits/country_widget.h"
#include "globals.h"

#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>

CountryMultiEdit::CountryMultiEdit(QList<AttributeMultiEdit *> &edits_out, QObject *parent)
: AttributeMultiEdit(edits_out, parent) {
    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_COUNTRY, CLASS_USER) + ":";
    apply_check->setText(label_text);

    combo = new QComboBox();
    country_combo_init(combo);

    connect(
        combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &CountryMultiEdit::edited);

    set_enabled(false);
}

CountryMultiEdit::CountryMultiEdit(QComboBox *combo_arg, QList<AttributeMultiEdit *> &edits_out, QObject *parent)
: AttributeMultiEdit(edits_out, parent) {
    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_COUNTRY, CLASS_USER) + ":";
    apply_check->setText(label_text);

    combo = combo_arg;
    country_combo_init(combo);

    connect(
        combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &CountryMultiEdit::edited);

    set_enabled(false);
}

void CountryMultiEdit::add_to_layout(QFormLayout *layout) {
    layout->addRow(apply_check, combo);
}

bool CountryMultiEdit::apply_internal(AdInterface &ad, const QString &target) {
    return country_combo_apply(combo, ad, target);
}

void CountryMultiEdit::set_enabled(const bool enabled) {
    combo->setEnabled(enabled);
}
