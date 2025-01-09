/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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
#include "filter_widget/ui_select_base_widget.h"

#include "adldap.h"
#include "globals.h"
#include "select_dialogs/select_container_dialog.h"
#include "utils.h"

SelectBaseWidget::SelectBaseWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::SelectBaseWidget();
    ui->setupUi(this);

    const QString domain_dn = g_adconfig->domain_dn();
    const QString domain_name = dn_get_name(domain_dn);
    ui->combo->addItem(domain_name, domain_dn);

    connect(
        ui->browse_button, &QAbstractButton::clicked,
        this, &SelectBaseWidget::open_browse_dialog);
}

SelectBaseWidget::~SelectBaseWidget() {
    delete ui;
}

void SelectBaseWidget::set_default_base(const QString &default_base) {
    const QString name = dn_get_name(default_base);
    ui->combo->addItem(name, default_base);

    // Select default base in combo
    const int last_index = ui->combo->count() - 1;
    ui->combo->setCurrentIndex(last_index);
}

QString SelectBaseWidget::get_base() const {
    const int index = ui->combo->currentIndex();
    const QVariant item_data = ui->combo->itemData(index);

    return item_data.toString();
}

void SelectBaseWidget::open_browse_dialog() {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    auto browse_dialog = new SelectContainerDialog(ad, this);
    browse_dialog->open();

    connect(
        browse_dialog, &QDialog::accepted,
        this,
        [this, browse_dialog]() {
            const QString selected = browse_dialog->get_selected();
            const QString name = dn_get_name(selected);

            const int added_base_index = ui->combo->findText(name);
            const bool base_already_added = (added_base_index != -1);

            if (base_already_added) {
                ui->combo->setCurrentIndex(added_base_index);
            } else {
                ui->combo->addItem(name, selected);

                // Select newly added search base in combobox
                const int new_base_index = ui->combo->count() - 1;
                ui->combo->setCurrentIndex(new_base_index);
            }
        });
}

QVariant SelectBaseWidget::save_state() const {
    const QString base = ui->combo->currentData().toString();
    return QVariant(base);
}

void SelectBaseWidget::restore_state(const QVariant &state_variant) {
    const QString base = state_variant.toString();
    const QString base_name = dn_get_name(base);
    ui->combo->clear();
    ui->combo->addItem(base_name, base);
}
