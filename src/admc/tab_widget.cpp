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

#include "tab_widget.h"
#include "ui_tab_widget.h"

TabWidget::TabWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::TabWidget();
    ui->setupUi(this);

    connect(
        ui->list_widget, &QListWidget::currentRowChanged,
        this, &TabWidget::on_list_current_row_changed);
}

TabWidget::~TabWidget() {
    delete ui;
}

QWidget *TabWidget::get_current_tab() const {
    QWidget *out = ui->stacked_widget->currentWidget();

    return out;
}

void TabWidget::add_tab(QWidget *tab, const QString &title) {
    ui->list_widget->addItem(title);
    ui->stacked_widget->addWidget(tab);
}

void TabWidget::on_list_current_row_changed(int index) {
    QWidget *prev_tab = ui->stacked_widget->currentWidget();

    ui->stacked_widget->setCurrentIndex(index);

    QWidget *new_tab = ui->stacked_widget->currentWidget();

    emit current_changed(prev_tab, new_tab);
}
