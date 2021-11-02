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

#include "edit_query_item_widget.h"
#include "ui_edit_query_item_widget.h"

#include "ad_filter.h"
#include "filter_widget/filter_dialog.h"
#include "globals.h"

EditQueryItemWidget::EditQueryItemWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::EditQueryItemWidget();
    ui->setupUi(this);

    filter_dialog = new FilterDialog(this);
    filter_dialog->add_classes(g_adconfig, filter_classes, filter_classes);

    ui->select_base_widget->init(g_adconfig);

    connect(
        ui->edit_filter_button, &QPushButton::clicked,
        filter_dialog, &QDialog::open);
    connect(
        filter_dialog, &QDialog::accepted,
        this, &EditQueryItemWidget::update_filter_display);
    update_filter_display();
}

EditQueryItemWidget::~EditQueryItemWidget() {
    delete ui;
}

QString EditQueryItemWidget::name() const {
    return ui->name_edit->text();
}

QString EditQueryItemWidget::description() const {
    return ui->description_edit->text();
}

QString EditQueryItemWidget::filter() const {
    return filter_dialog->get_filter();
}

QString EditQueryItemWidget::base() const {
    return ui->select_base_widget->get_base();
}

bool EditQueryItemWidget::scope_is_children() const {
    return !ui->scope_checkbox->isChecked();
}

QByteArray EditQueryItemWidget::filter_state() const {
    QHash<QString, QVariant> state;

    state["select_base_widget"] = ui->select_base_widget->save_state();
    state["filter_widget"] = filter_dialog->save_state();
    state["filter"] = filter();

    QByteArray out;
    QDataStream state_stream(&out, QIODevice::WriteOnly);
    state_stream << state;

    return out;
}

void EditQueryItemWidget::clear() {
    const QString name = QString();
    const QString description = QString();
    const bool scope_is_children = false;
    const QByteArray filter_state = QByteArray();

    set_data(name, description, scope_is_children, filter_state);
}

void EditQueryItemWidget::set_data(const QString &name, const QString &description, const bool scope_is_children, const QByteArray &filter_state) {
    QDataStream filter_state_stream(filter_state);
    QHash<QString, QVariant> state;
    filter_state_stream >> state;

    ui->select_base_widget->restore_state(state["select_base_widget"]);
    filter_dialog->restore_state(state["filter_widget"]);

    update_filter_display();

    ui->name_edit->setText(name);
    ui->description_edit->setText(description);
    ui->scope_checkbox->setChecked(!scope_is_children);
}

void EditQueryItemWidget::update_filter_display() {
    const QString filter_in_dialog = filter_dialog->get_filter();
    ui->filter_display->setPlainText(filter_in_dialog);
}
