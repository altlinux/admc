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

#include "console_widget/customize_columns_dialog.h"
#include "console_widget/customize_columns_dialog_p.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QPushButton>
#include <QScrollArea>
#include <QTreeView>
#include <QVBoxLayout>

CustomizeColumnsDialogPrivate::CustomizeColumnsDialogPrivate(CustomizeColumnsDialog *q)
: QObject(q) {
}

CustomizeColumnsDialog::CustomizeColumnsDialog(QTreeView *view_arg, const QList<int> &default_columns_arg, QWidget *parent)
: QDialog(parent) {
    d = new CustomizeColumnsDialogPrivate(this);

    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("Customize Columns"));

    d->view = view_arg;
    d->default_columns = default_columns_arg;

    QHeaderView *header = d->view->header();
    QAbstractItemModel *model = d->view->model();

    auto checkboxes_widget = new QWidget();

    auto checkboxes_layout = new QVBoxLayout();
    checkboxes_widget->setLayout(checkboxes_layout);

    for (int i = 0; i < header->count(); i++) {
        const QString column_name = model->headerData(i, Qt::Horizontal).toString();

        auto checkbox = new QCheckBox(column_name);
        const bool currently_hidden = header->isSectionHidden(i);
        checkbox->setChecked(!currently_hidden);

        d->checkbox_list.append(checkbox);
    }

    for (int i = 0; i < header->count(); i++) {
        auto checkbox = d->checkbox_list[i];
        checkboxes_layout->addWidget(checkbox);
    }

    auto scroll_area = new QScrollArea();
    scroll_area->setWidget(checkboxes_widget);

    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    auto cancel_button = button_box->addButton(QDialogButtonBox::Cancel);
    auto restore_defaults_button = button_box->addButton(QDialogButtonBox::RestoreDefaults);

    auto dialog_layout = new QVBoxLayout();
    setLayout(dialog_layout);

    dialog_layout->addWidget(scroll_area);
    dialog_layout->addWidget(button_box);

    connect(
        ok_button, &QPushButton::clicked,
        this, &CustomizeColumnsDialog::accept);
    connect(
        cancel_button, &QPushButton::clicked,
        this, &QDialog::reject);
    connect(
        restore_defaults_button, &QPushButton::clicked,
        d, &CustomizeColumnsDialogPrivate::restore_defaults);
}

void CustomizeColumnsDialog::accept() {
    QHeaderView *header = d->view->header();

    for (int i = 0; i < d->checkbox_list.size(); i++) {
        QCheckBox *checkbox = d->checkbox_list[i];
        const bool hidden = !checkbox->isChecked();
        header->setSectionHidden(i, hidden);
    }

    QDialog::accept();
}

void CustomizeColumnsDialogPrivate::restore_defaults() {
    for (int i = 0; i < checkbox_list.size(); i++) {
        QCheckBox *checkbox = checkbox_list[i];
        const bool hidden = !default_columns.contains(i);
        checkbox->setChecked(!hidden);
    }
}
