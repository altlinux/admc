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
#include "edit_query_item_widget_p.h"
#include "ui_edit_query_item_widget.h"
#include "ui_edit_query_item_filter_dialog.h"

#include "ad_filter.h"
#include "globals.h"
#include "console_impls/query_item_impl.h"
#include "filter_widget/filter_widget.h"
#include "filter_widget/select_base_widget.h"

#include <QModelIndex>

EditQueryItemWidget::EditQueryItemWidget()
: QWidget() {
    ui = new Ui::EditQueryItemWidget();
    ui->setupUi(this);

    dialog = new EditQueryItemFilterDialog(this);

    ui->select_base_widget->init(g_adconfig);

    connect(
        ui->edit_filter_button, &QPushButton::clicked,
        dialog, &QDialog::open);
    connect(
        dialog, &QDialog::accepted,
        this, &EditQueryItemWidget::update_filter_display);
    update_filter_display();
}

void EditQueryItemWidget::load(const QModelIndex &index) {
    QByteArray filter_state = index.data(QueryItemRole_FilterState).toByteArray();
    QDataStream filter_state_stream(filter_state);
    QHash<QString, QVariant> state;
    filter_state_stream >> state;

    ui->select_base_widget->restore_state(state["select_base_widget"]);
    dialog->ui->filter_widget->restore_state(state["filter_widget"]);

    update_filter_display();

    const QString name = index.data(Qt::DisplayRole).toString();
    ui->name_edit->setText(name);

    const QString description = index.data(QueryItemRole_Description).toString();
    ui->description_edit->setText(description);

    const bool scope_is_children = index.data(QueryItemRole_ScopeIsChildren).toBool();
    ui->scope_checkbox->setChecked(!scope_is_children);
}

void EditQueryItemWidget::save(QString &name, QString &description, QString &filter, QString &base, bool &scope_is_children, QByteArray &filter_state) const {
    name = ui->name_edit->text();
    description = ui->description_edit->text();
    filter = dialog->ui->filter_widget->get_filter();
    base = ui->select_base_widget->get_base();
    scope_is_children = !ui->scope_checkbox->isChecked();

    filter_state = [&]() {
        QHash<QString, QVariant> state;

        state["select_base_widget"] = ui->select_base_widget->save_state();
        state["filter_widget"] = dialog->ui->filter_widget->save_state();
        state["filter"] = filter;

        QByteArray out;
        QDataStream state_stream(&out, QIODevice::WriteOnly);
        state_stream << state;

        return out;
    }();
}

void EditQueryItemWidget::update_filter_display() {
    const QString filter = dialog->ui->filter_widget->get_filter();
    ui->filter_display->setPlainText(filter);
}

EditQueryItemFilterDialog::EditQueryItemFilterDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::EditQueryItemFilterDialog();
    ui->setupUi(this);

    ui->filter_widget->add_classes(g_adconfig, filter_classes);
}
