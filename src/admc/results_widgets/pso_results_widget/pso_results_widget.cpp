/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#include "pso_results_widget.h"
#include "ad_interface.h"
#include "utils.h"
#include "console_impls/object_impl/object_impl.h"
#include "pso_edit_widget.h"
#include "../ui_results_widget_base.h"

#include <QModelIndex>

PSOResultsWidget::PSOResultsWidget(QWidget *parent) :
    ResultsWidgetBase(parent), pso_edit_widget(new PSOEditWidget(this)) {

    ui->verticalLayout->addWidget(pso_edit_widget);

    ui->edit_button->setDisabled(true);
    ui->cancel_button->setDisabled(true);
    ui->apply_button->setDisabled(true);
    pso_edit_widget->set_read_only(true);
}

void PSOResultsWidget::update(const QModelIndex &index) {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    AdObject pso = ad.search_object(index.data(ObjectRole_DN).toString());
    update(pso);
}

void PSOResultsWidget::update(const AdObject &pso) {
    ResultsWidgetBase::update(pso);
    saved_object = pso;
    pso_edit_widget->update(pso);
    pso_edit_widget->set_read_only(true);
}

void PSOResultsWidget::on_apply() {
    if (changed_attrs().isEmpty()) {
        set_editable(false);
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, this)) {
        on_cancel_edit();
        return;
    }

    const QString &pso_dn = saved_object.get_dn();
    QStringList changed_attr_list = changed_attrs();
    auto settings_values = pso_edit_widget->pso_settings_values();
    for (const QString &attribute : changed_attr_list) {
        ad.attribute_replace_values(pso_dn, attribute, settings_values.value(attribute, QList<QByteArray>()));
    }

    saved_object = ad.search_object(pso_dn);

    set_editable(false);
}

void PSOResultsWidget::on_edit() {
    set_editable(true);
}

void PSOResultsWidget::on_cancel_edit() {
    pso_edit_widget->update(saved_object);
    set_editable(false);
}

QStringList PSOResultsWidget::changed_attrs() {
    QStringList attrs;
    auto new_values = pso_edit_widget->pso_settings_values();
    for (const QString &attribute : new_values.keys()) {
        if (saved_object.get_values(attribute) != new_values.value(attribute, QList<QByteArray>())) {
            attrs.append(attribute);
        }
    }

    return attrs;
}

void PSOResultsWidget::set_editable(bool is_editable) {
    ResultsWidgetBase::set_editable(is_editable);
    pso_edit_widget->set_read_only(!is_editable);
}
