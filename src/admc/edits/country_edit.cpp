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

#include "edits/country_edit.h"

#include "adldap.h"
#include "edits/country_widget.h"
#include "globals.h"
#include "status.h"
#include "tabs/address_tab.h"
#include "utils.h"

#include <QComboBox>
#include <QDebug>
#include <QFile>
#include <QFormLayout>
#include <QHash>
#include <QVBoxLayout>
#include <algorithm>

CountryEdit::CountryEdit(QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    country_widget = new CountryWidget();

    connect(
        country_widget, &CountryWidget::edited,
        this, &CountryEdit::edited);
}

void CountryEdit::load_internal(AdInterface &ad, const AdObject &object) {
    country_widget->load(object);
}

void CountryEdit::set_read_only(const bool read_only) {
    country_widget->set_enabled(!read_only);
}

void CountryEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_COUNTRY, CLASS_USER) + ":";
    layout->addRow(label_text, country_widget);
}

bool CountryEdit::apply(AdInterface &ad, const QString &dn) const {
    return country_widget->apply(ad, dn);
}
