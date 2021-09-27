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

#include "filter_widget/select_base_widget.h"

#include "adldap.h"
#include "select_container_dialog.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>

// TODO: missing "Entire directory" in search base combo. Not 100% sure what it's supposed to be, the tippy-top domain? Definitely need it for work with multiple domains.

SelectBaseWidget::SelectBaseWidget(QWidget *parent)
: QWidget(parent) {
    combo = new QComboBox();
    combo->setMinimumWidth(200);

    auto browse_button = new QPushButton(tr("Browse..."));
    browse_button->setAutoDefault(false);
    browse_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    auto layout = new QHBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(combo);
    layout->addWidget(browse_button);

    connect(
        browse_button, &QAbstractButton::clicked,
        this, &SelectBaseWidget::browse);
}
void SelectBaseWidget::init(AdConfig *adconfig, const QString &default_base) {
    const QString domain_head = adconfig->domain_head();

    auto add_base_to_combo = [this](const QString dn) {
        const QString name = dn_get_name(dn);
        combo->addItem(name, dn);
    };

    if (default_base == domain_head || default_base.isEmpty()) {
        add_base_to_combo(domain_head);
    } else {
        add_base_to_combo(domain_head);
        add_base_to_combo(default_base);
    }

    const int last_index = combo->count() - 1;
    combo->setCurrentIndex(last_index);
}

QString SelectBaseWidget::get_base() const {
    const int index = combo->currentIndex();
    const QVariant item_data = combo->itemData(index);

    return item_data.toString();
}

void SelectBaseWidget::browse() {
    auto dialog = new SelectContainerDialog(this);

    connect(
        dialog, &SelectContainerDialog::accepted,
        [this, dialog]() {
            const QString selected = dialog->get_selected();
            const QString name = dn_get_name(selected);

            combo->addItem(name, selected);

            // Select newly added search base in combobox
            const int new_base_index = combo->count() - 1;
            combo->setCurrentIndex(new_base_index);
        });

    dialog->open();
}

QVariant SelectBaseWidget::save_state() const {
    const QString base = combo->currentData().toString();
    return QVariant(base);
}

void SelectBaseWidget::restore_state(const QVariant &state_variant) {
    const QString base = state_variant.toString();
    const QString base_name = dn_get_name(base);
    combo->clear();
    combo->addItem(base_name, base);
}
