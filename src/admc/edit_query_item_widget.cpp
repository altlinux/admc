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

#include "ad_filter.h"
#include "console_types/console_query.h"
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

EditQueryItemWidget::EditQueryItemWidget()
: QWidget() {
    setMinimumWidth(400);

    select_base_widget = new SelectBaseWidget();

    filter_widget = new FilterWidget(filter_classes);

    name_edit = new QLineEdit();
    name_edit->setText("New query");

    description_edit = new QLineEdit();

    scope_checkbox = new QCheckBox(tr("Include subcontainers"));

    filter_display = new QTextEdit();
    filter_display->setReadOnly(true);

    auto edit_filter_button = new QPushButton(tr("Edit"));

    auto edit_filter_dialog = new QDialog(this);
    edit_filter_dialog->setWindowTitle("Edit filter");

    auto dialog_button_box = new QDialogButtonBox();
    dialog_button_box->addButton(QDialogButtonBox::Ok);
    dialog_button_box->addButton(QDialogButtonBox::Cancel);

    auto dialog_layout = new QVBoxLayout();
    edit_filter_dialog->setLayout(dialog_layout);
    dialog_layout->addWidget(filter_widget);
    dialog_layout->addWidget(dialog_button_box);

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
        dialog_button_box, &QDialogButtonBox::accepted,
        edit_filter_dialog, &QDialog::accept);
    connect(
        dialog_button_box, &QDialogButtonBox::rejected,
        edit_filter_dialog, &QDialog::reject);

    connect(
        edit_filter_button, &QPushButton::clicked,
        edit_filter_dialog, &QDialog::open);
    connect(
        edit_filter_dialog, &QDialog::accepted,
        this, &EditQueryItemWidget::update_filter_display);
    update_filter_display();
}

void EditQueryItemWidget::load(const QModelIndex &index) {
    QByteArray filter_state = index.data(QueryItemRole_FilterState).toByteArray();
    QDataStream filter_state_stream(filter_state);
    QHash<QString, QVariant> state;
    filter_state_stream >> state;

    const QHash<QString, QVariant> select_base_widget_state = state["select_base_widget"].toHash();
    const QHash<QString, QVariant> filter_widget_state = state["filter_widget"].toHash();

    select_base_widget->load_state(select_base_widget_state);
    filter_widget->load_state(filter_widget_state);

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
    filter = filter_widget->get_filter();
    base = select_base_widget->get_base();
    scope_is_children = !scope_checkbox->isChecked();

    filter_state = [&]() {
        QHash<QString, QVariant> select_base_widget_state;
        QHash<QString, QVariant> filter_widget_state;

        select_base_widget->save_state(select_base_widget_state);
        filter_widget->save_state(filter_widget_state);

        QHash<QString, QVariant> state;
        state["select_base_widget"] = select_base_widget_state;
        state["filter_widget"] = filter_widget_state;
        state["filter"] = filter;

        QByteArray out;
        QDataStream state_stream(&out, QIODevice::WriteOnly);
        state_stream << state;

        return out;
    }();
}

void EditQueryItemWidget::update_filter_display() {
    const QString filter = filter_widget->get_filter();
    filter_display->setPlainText(filter);
}
