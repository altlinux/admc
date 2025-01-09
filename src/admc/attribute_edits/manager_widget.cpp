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

#include "attribute_edits/manager_widget.h"
#include "attribute_edits/ui_manager_widget.h"

#include "adldap.h"
#include "globals.h"
#include "properties_widgets/properties_dialog.h"
#include "select_dialogs/select_object_dialog.h"
#include "utils.h"

ManagerWidget::ManagerWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::ManagerWidget();
    ui->setupUi(this);

    ui->manager_display->setReadOnly(true);

    connect(
        ui->change_button, &QPushButton::clicked,
        this, &ManagerWidget::on_change);
    connect(
        ui->properties_button, &QPushButton::clicked,
        this, &ManagerWidget::on_properties);
    connect(
        ui->clear_button, &QPushButton::clicked,
        this, &ManagerWidget::on_clear);
}

ManagerWidget::~ManagerWidget() {
    delete ui;
}

void ManagerWidget::set_attribute(const QString &attribute) {
    manager_attribute = attribute;
}

void ManagerWidget::load(const AdObject &object) {
    const QString manager = object.get_string(manager_attribute);

    load_value(manager);
}

bool ManagerWidget::apply(AdInterface &ad, const QString &dn) const {
    const bool success = ad.attribute_replace_string(dn, manager_attribute, current_value);

    return success;
}

QString ManagerWidget::get_manager() const {
    return current_value;
}

void ManagerWidget::reset() {
    ui->manager_display->setText(QString());
}

void ManagerWidget::on_change() {
    auto dialog = new SelectObjectDialog({CLASS_USER, CLASS_CONTACT}, SelectObjectDialogMultiSelection_No, ui->manager_display);
    dialog->setWindowTitle(tr("Change Manager"));

    connect(
        dialog, &SelectObjectDialog::accepted,
        this,
        [this, dialog]() {
            const QList<QString> selected = dialog->get_selected();

            load_value(selected[0]);

            emit edited();
        });

    dialog->open();
}

void ManagerWidget::on_properties() {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    PropertiesDialog::open_for_target(ad, current_value);
}

void ManagerWidget::on_clear() {
    load_value(QString());

    emit edited();
}

void ManagerWidget::load_value(const QString &value) {
    current_value = value;

    const QString name = dn_get_name(current_value);
    ui->manager_display->setText(name);

    const bool have_manager = !current_value.isEmpty();
    ui->properties_button->setEnabled(have_manager);
    ui->clear_button->setEnabled(have_manager);
}
