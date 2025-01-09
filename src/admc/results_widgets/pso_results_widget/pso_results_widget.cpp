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
#include "ui_pso_results_widget.h"
#include "ad_interface.h"
#include "utils.h"
#include "console_impls/object_impl.h"

#include <QModelIndex>
// #include <QDebug>


PSOResultsWidget::PSOResultsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PSOResultsWidget) {
    ui->setupUi(this);

    connect(ui->apply_button, &QPushButton::clicked, this, &PSOResultsWidget::on_apply);
    connect(ui->cancel_button, &QPushButton::clicked, this, &PSOResultsWidget::on_cancel);
    connect(ui->edit_button, &QPushButton::clicked, this, &PSOResultsWidget::on_edit);
}

PSOResultsWidget::~PSOResultsWidget() {
    delete ui;
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
    saved_pso_object = pso;
    ui->pso_edit_widget->update(pso);
    ui->edit_button->setDisabled(false);
    ui->cancel_button->setDisabled(true);
    ui->apply_button->setDisabled(true);
    ui->pso_edit_widget->set_read_only(true);
}

void PSOResultsWidget::on_apply() {
    if (changed_setting_attrs().isEmpty()) {
        set_editable(false);
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, this)) {
        on_cancel();
        return;
    }

    const QString &pso_dn = saved_pso_object.get_dn();
    QStringList changed_attrs = changed_setting_attrs();
    auto settings_values = ui->pso_edit_widget->pso_settings_values();
    for (const QString &attribute : changed_attrs) {
        ad.attribute_replace_values(pso_dn, attribute, settings_values[attribute]);
    }

    saved_pso_object = ad.search_object(pso_dn);

    set_editable(false);
}

void PSOResultsWidget::on_edit() {
    set_editable(true);
}

void PSOResultsWidget::on_cancel() {
    ui->pso_edit_widget->update(saved_pso_object);
    set_editable(false);
}

QStringList PSOResultsWidget::changed_setting_attrs() {
    QStringList attrs;
    auto new_values = ui->pso_edit_widget->pso_settings_values();
    for (const QString &attribute : new_values.keys()) {
        if (saved_pso_object.get_values(attribute) != new_values[attribute]) {
            attrs.append(attribute);
        }
    }

    return attrs;
}

void PSOResultsWidget::set_editable(bool is_editable) {
    ui->pso_edit_widget->set_read_only(!is_editable);
    ui->edit_button->setDisabled(is_editable);
    ui->cancel_button->setDisabled(!is_editable);
    ui->apply_button->setDisabled(!is_editable);
}
