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

#include "filter_widget/filter_widget.h"
#include "filter_widget/ui_filter_widget.h"

#include "filter_widget/filter_widget_advanced_tab.h"
#include "filter_widget/filter_widget_normal_tab.h"
#include "filter_widget/filter_widget_simple_tab.h"

#include <QDebug>

FilterWidget::FilterWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::FilterWidget();
    ui->setupUi(this);
}

FilterWidget::~FilterWidget() {
    delete ui;
}

void FilterWidget::set_classes(const QList<QString> &class_list, const QList<QString> &selected_list) {
    ui->simple_tab->set_classes(class_list, selected_list);
    ui->normal_tab->set_classes(class_list, selected_list);
}

void FilterWidget::enable_filtering_all_classes() {
    ui->simple_tab->enable_filtering_all_classes();
    ui->normal_tab->enable_filtering_all_classes();
}

QString FilterWidget::get_filter() const {
    const FilterWidgetTab *current_tab = dynamic_cast<FilterWidgetTab *>(ui->tab_widget->currentWidget());

    if (current_tab) {
        return current_tab->get_filter();
    } else {
        qDebug() << "Inserted a non FilterWidgetTab into FilterWidget";
        return QString();
    }
}

QVariant FilterWidget::save_state() const {
    QHash<QString, QVariant> state;

    state["current_tab_index"] = ui->tab_widget->currentIndex();
    state["simple_state"] = ui->simple_tab->save_state();
    state["normal_state"] = ui->normal_tab->save_state();
    state["advanced_state"] = ui->advanced_tab->save_state();

    return QVariant(state);
}

void FilterWidget::restore_state(const QVariant &state_variant) {
    const QHash<QString, QVariant> state = state_variant.toHash();

    ui->tab_widget->setCurrentIndex(state["current_tab_index"].toInt());
    ui->simple_tab->restore_state(state["simple_state"]);
    ui->normal_tab->restore_state(state["normal_state"]);
    ui->advanced_tab->restore_state(state["advanced_state"]);
}

void FilterWidget::clear() {
    ui->simple_tab->clear();
    ui->normal_tab->clear();
    ui->advanced_tab->clear();
}
