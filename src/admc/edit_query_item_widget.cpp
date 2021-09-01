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

#include "ad_filter.h"
#include "globals.h"
#include "console_impls/query_item_impl.h"
#include "filter_widget/filter_widget.h"
#include "filter_widget/select_base_widget.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QModelIndex>

EditQueryItemWidget::EditQueryItemWidget()
: QWidget() {
    setMinimumWidth(400);

    dialog = new EditQueryItemFilterDialog(this);

    select_base_widget = new SelectBaseWidget(g_adconfig);
    select_base_widget->setObjectName("select_base_widget");

    name_edit = new QLineEdit();
    name_edit->setObjectName("name_edit");
    name_edit->setText("New query");

    description_edit = new QLineEdit();
    description_edit->setObjectName("description_edit");

    scope_checkbox = new QCheckBox(tr("Recursive search"));
    scope_checkbox->setObjectName("scope_checkbox");

    filter_display = new QTextEdit();
    filter_display->setReadOnly(true);
    filter_display->setObjectName("filter_display");

    auto edit_filter_button = new QPushButton(tr("Edit..."));
    edit_filter_button->setObjectName("edit_filter_button");

    auto form_layout = new QFormLayout();
    form_layout->addRow(tr("Name:"), name_edit);
    form_layout->addRow(tr("Description:"), description_edit);
    form_layout->addRow(tr("Search in:"), select_base_widget);
    form_layout->addRow(scope_checkbox);
    form_layout->addRow(new QLabel(tr("Filter:")));

    auto filter_layout = new QHBoxLayout();
    filter_layout->addWidget(filter_display);
    filter_layout->addWidget(edit_filter_button);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(form_layout);
    layout->addLayout(filter_layout);

    connect(
        edit_filter_button, &QPushButton::clicked,
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

    select_base_widget->restore_state(state["select_base_widget"]);
    dialog->filter_widget->restore_state(state["filter_widget"]);

    update_filter_display();

    const QString name = index.data(Qt::DisplayRole).toString();
    name_edit->setText(name);

    const QString description = index.data(QueryItemRole_Description).toString();
    description_edit->setText(description);

    const bool scope_is_children = index.data(QueryItemRole_ScopeIsChildren).toBool();
    scope_checkbox->setChecked(!scope_is_children);
}

void EditQueryItemWidget::save(QString &name, QString &description, QString &filter, QString &base, bool &scope_is_children, QByteArray &filter_state) const {
    name = name_edit->text();
    description = description_edit->text();
    filter = dialog->filter_widget->get_filter();
    base = select_base_widget->get_base();
    scope_is_children = !scope_checkbox->isChecked();

    filter_state = [&]() {
        QHash<QString, QVariant> state;

        state["select_base_widget"] = select_base_widget->save_state();
        state["filter_widget"] = dialog->filter_widget->save_state();
        state["filter"] = filter;

        QByteArray out;
        QDataStream state_stream(&out, QIODevice::WriteOnly);
        state_stream << state;

        return out;
    }();
}

void EditQueryItemWidget::update_filter_display() {
    const QString filter = dialog->filter_widget->get_filter();
    filter_display->setPlainText(filter);
}

EditQueryItemFilterDialog::EditQueryItemFilterDialog(QWidget *parent)
: QDialog(parent) {
    setWindowTitle(tr("Edit Filter"));

    filter_widget = new FilterWidget(g_adconfig, filter_classes);

    auto button_box = new QDialogButtonBox();
    button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(filter_widget);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
}
