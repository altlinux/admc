/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2025-2026 BaseALT Ltd.
 * Copyright (C) 2025 Semyon Knyazev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include <QLabel>

#include "ad_interface.h"
#include "ad_object.h"
#include "pso_applied_edit.h"

PSOAppliedEdit::PSOAppliedEdit(QLabel *label, QObject *parent)
: AttributeEdit(parent), applied_pso_label(label) {

}

void PSOAppliedEdit::load(AdInterface &ad, const AdObject &object) {
    applied_pso_label->setText(tr("Default"));

    const QStringList pso_dn_list = object.get_strings(ATTRIBUTE_PSO_APPLIED);
    if (pso_dn_list.isEmpty()) {
        return;
    }

    AdObject applied_pso_obj;
    int precedence = 0;
    for (const QString &pso_dn : pso_dn_list) {
        const AdObject pso_obj = ad.search_object(pso_dn, {ATTRIBUTE_NAME, ATTRIBUTE_MS_DS_PASSWORD_SETTINGS_PRECEDENCE,
                                                           ATTRIBUTE_PSO_APPLIES_TO});
        if (pso_obj.is_empty()) {
            continue;
        }

        int obj_precedence = pso_obj.get_int(ATTRIBUTE_MS_DS_PASSWORD_SETTINGS_PRECEDENCE);
        if (obj_precedence > precedence) {
            applied_pso_obj = pso_obj;
            precedence = obj_precedence;
        }
    }

    if (applied_pso_obj.is_empty()) {
        applied_pso_label->setText(tr("Not found"));
        return;
    }

    QString label_text = applied_pso_obj.get_string(ATTRIBUTE_NAME);
    const QStringList dn_applied_list = applied_pso_obj.get_strings(ATTRIBUTE_PSO_APPLIES_TO);
    if (dn_applied_list.contains(object.get_dn())) {
        label_text += tr(" (directly)");
    }
    else {
        label_text += tr(" (via group membership)");
    }

    applied_pso_label->setText(label_text);
}
