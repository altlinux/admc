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

#include "select_object_dialog.h"
#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"
#include "console_types/console_object.h"
#include "find_select_object_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTreeView>
#include <QPushButton>
#include <QItemSelectionModel>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QStandardItemModel>

SelectObjectDialog::SelectObjectDialog(QList<QString> classes_arg, SelectObjectDialogMultiSelection multi_selection_arg, QWidget *parent)
: QDialog(parent)
{
    classes = classes_arg;
    multi_selection = multi_selection_arg;

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Select dialog");

    resize(400, 300);

    model = new QStandardItemModel(this);

    const QList<QString> header_labels = console_object_header_labels();
    model->setHorizontalHeaderLabels(header_labels);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->sortByColumn(0, Qt::AscendingOrder);

    view->setModel(model);

    auto add_button = new QPushButton(tr("Add"));
    auto remove_button = new QPushButton(tr("Remove"));

    auto button_box = new QDialogButtonBox();
    button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);

    auto list_buttons_layout = new QHBoxLayout();
    list_buttons_layout->addWidget(add_button);
    list_buttons_layout->addWidget(remove_button);
    list_buttons_layout->addStretch(1);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(view);
    layout->addLayout(list_buttons_layout);
    layout->addWidget(button_box);

    enable_widget_on_selection(remove_button, view);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);

    connect(
        add_button, &QPushButton::clicked,
        this, &SelectObjectDialog::open_find_dialog);
    connect(
        add_button, &QPushButton::clicked,
        this, &SelectObjectDialog::remove_from_list);
}

QList<QString> SelectObjectDialog::get_selected() const {
    QSet<QString> selected_dns;

    for (int row = 0; row < model->rowCount(); row++) {
        const QModelIndex index = model->index(row, 0);
        const QString dn = index.data(ObjectRole_DN).toString();

        selected_dns.insert(dn);
    }

    return selected_dns.toList();
}

void SelectObjectDialog::accept() {
    const QList<QString> selected = get_selected();

    const bool selected_multiple_when_single_selection = (multi_selection == SelectObjectDialogMultiSelection_No && selected.size() > 1);
    if (selected_multiple_when_single_selection) {
        QMessageBox::warning(this, tr("Error"), tr("This selection accepts only one object. Remove extra objects to proceed."));
    } else {
        QDialog::accept();
    }
}

void SelectObjectDialog::open_find_dialog() {
    auto dialog = new FindSelectObjectDialog(classes, this);

    connect(
        dialog, &FindSelectObjectDialog::accepted,
        [this, dialog]() {
            // Add objects selected in find dialog to select
            // objects list
            // NOTE: adding selected objects could've been done by just getting selected DN's, BUT that would involve performing a search for each of the objects to get necessary attributes to create rows. Object amounts can be potentially huge, so that would really hurt performance. So, instead we're getting raw rows of items even though it's kinda messy. In summary: DN's = simple, very slow; items = messy, fast.
            const QList<QString> current_selected = get_selected();

            const QList<QList<QStandardItem *>> selected_rows = dialog->get_selected_rows();

            for (auto row : selected_rows) {
                const bool model_contains_object = [=]() {
                    QStandardItem *dn_item = row[0];
                    const QString dn = dn_item->text();

                    return current_selected.contains(dn);
                }();
                
                if (!model_contains_object) {
                    model->appendRow(row);
                } else {
                    for (auto item : row) {
                        delete item;
                    }
                }
            }
        });

    dialog->open();
}

void SelectObjectDialog::remove_from_list() {
    const QItemSelectionModel *selection_model = view->selectionModel();
    const QList<QModelIndex> selected = selection_model->selectedRows();

    for (auto index : selected) {
        model->removeRows(index.row(), 1);
    }
}

void SelectObjectDialog::showEvent(QShowEvent *event) {
    resize_columns(view,
    {
        {0, 0.4},
        {1, 0.4},
    });
}
