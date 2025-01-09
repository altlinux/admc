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

#include "tab_widget.h"
#include "ui_tab_widget.h"

// TODO: a lot of code has grown here that is used only
// by properties dialog. Other places(fsmo and multi
// properties) only use add_tab() and connection of
// list widget to stacked widget. Should separate those
// two into free f-ns. Properties dialog should
// implement all of the extra stuff inside it. UI files
// for users of this file will need to be updated to
// contain list/stacked widget.

TabWidget::TabWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::TabWidget();
    ui->setupUi(this);

    auto_switch_tab = true;
    ignore_current_row_signal = false;

    connect(
        ui->list_widget, &QListWidget::currentRowChanged,
        this, &TabWidget::on_list_current_row_changed);
}

TabWidget::~TabWidget() {
    delete ui;
}

QWidget *TabWidget::get_tab(const int index) const {
    QWidget *out = ui->stacked_widget->widget(index);

    return out;
}

QWidget *TabWidget::get_current_tab() const {
    QWidget *out = ui->stacked_widget->currentWidget();

    return out;
}

void TabWidget::set_current_tab(const int index) {
    ignore_current_row_signal = true;
    ui->list_widget->setCurrentRow(index, QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent);
    ignore_current_row_signal = false;
    ui->stacked_widget->setCurrentIndex(index);
}

void TabWidget::add_tab(QWidget *tab, const QString &title) {
    ui->list_widget->addItem(title);

    ui->stacked_widget->addWidget(tab);
}

void TabWidget::enable_auto_switch_tab(const bool enabled) {
    auto_switch_tab = enabled;
}

void TabWidget::on_list_current_row_changed(int index) {
    if (ignore_current_row_signal) {
        return;
    }

    // NOTE: since tab wasn't change yet in stacked
    // widget, we may use it to get previous tab index
    const int prev = ui->stacked_widget->currentIndex();

    if (auto_switch_tab) {
        ui->stacked_widget->setCurrentIndex(index);
    } else {
        // Restore prev index so current in list widget
        // is the same as in stacked widget
        ignore_current_row_signal = true;
        ui->list_widget->setCurrentRow(prev, QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent);
        ignore_current_row_signal = false;
    }

    emit current_changed(prev, index);
}
