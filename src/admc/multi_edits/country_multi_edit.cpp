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

#include "multi_edits/country_multi_edit.h"

#include "adldap.h"
#include "globals.h"
#include "edits/country_widget.h"

#include <QFormLayout>
#include <QLabel>

CountryMultiEdit::CountryMultiEdit(QList<AttributeMultiEdit *> &edits_out, QObject *parent)
: AttributeMultiEdit(edits_out, parent)
{
    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_COUNTRY, CLASS_USER) + ":";
    label->setText(label_text);

    country_widget = new CountryWidget();

    connect(
        country_widget, &CountryWidget::edited,
        this, &CountryMultiEdit::edited);

    set_enabled(false);
}

void CountryMultiEdit::add_to_layout(QFormLayout *layout) {
    layout->addRow(check_and_label_wrapper, country_widget);
}

bool CountryMultiEdit::apply_internal(AdInterface &ad, const QString &target) {
    return country_widget->apply(ad, target);
}

void CountryMultiEdit::set_enabled(const bool enabled) {
    country_widget->set_enabled(enabled);
}
