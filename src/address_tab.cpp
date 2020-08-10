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

#include "address_tab.h"
#include "ad_interface.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QDateTime>
#include <QButtonGroup>
#include <QCalendarWidget>
#include <QDialog>

AddressTab::AddressTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    title = tr("Address");

    // TODO: don't know why, but if i just have hbox as top layout, the widgets are misaligned
    const auto top_layout = new QVBoxLayout(this);
    // top_layout->addWidget(name_label);

    const auto attributes_layout = new QHBoxLayout();
    top_layout->insertLayout(-1, attributes_layout);

    const auto label_layout = new QVBoxLayout();
    const auto edit_layout = new QVBoxLayout();
    attributes_layout->insertLayout(-1, label_layout);
    attributes_layout->insertLayout(-1, edit_layout);

    auto make_line_edit =
    [this, label_layout, edit_layout](const QString &attribute, const QString &label_text) {
        add_attribute_edit(attribute, label_text, label_layout, edit_layout);
    };

    make_line_edit(ATTRIBUTE_STREET, tr("Street:"));
    make_line_edit(ATTRIBUTE_PO_BOX, tr("P.O. Box:"));
    make_line_edit(ATTRIBUTE_CITY, tr("City"));
    make_line_edit(ATTRIBUTE_STATE, tr("State/province:"));
    make_line_edit(ATTRIBUTE_POSTAL_CODE, tr("Postal code:"));
    // make_line_edit(ATTRIBUTE_COUNTRY, tr("Country:"));
}

void AddressTab::reload() {
    emit reloaded();
}

bool AddressTab::accepts_target() const {
    return AdInterface::instance()->is_user(target());
}
