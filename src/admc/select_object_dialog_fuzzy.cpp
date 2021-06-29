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

#include "select_object_dialog_fuzzy.h"

#include "adldap.h"
#include "console_types/console_object.h"
#include "globals.h"
#include "utils.h"
#include "console_types/console_object.h"
#include "filter_widget/select_classes_widget.h"
#include "filter_widget/select_base_widget.h"

#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>

SelectObjectDialogFuzzy::SelectObjectDialogFuzzy(const QList<QString> classes, QWidget *parent)
: QDialog(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Select dialog");

    resize(600, 500);

    select_classes = new SelectClassesWidget(classes);

    select_base_widget = new SelectBaseWidget();

    edit = new QLineEdit();

    auto add_button = new QPushButton(tr("Add"));
    add_button->setDefault(true);

    model = new QStandardItemModel(this);

    const QList<QString> header_labels = console_object_header_labels();
    model->setHorizontalHeaderLabels(header_labels);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->sortByColumn(0, Qt::AscendingOrder);

    view->setModel(model);

    auto button_box = new QDialogButtonBox();
    auto ok_button = button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);
    ok_button->setDefault(false);

    auto layout = new QFormLayout();
    setLayout(layout);
    layout->addRow(tr("Classes:"), select_classes);
    layout->addRow(tr("Search in:"), select_base_widget);
    layout->addRow(tr("Name:"), edit);
    layout->addRow(add_button);
    layout->addRow(view);
    layout->addRow(button_box);

    connect(
        add_button, &QPushButton::clicked,
        this, &SelectObjectDialogFuzzy::on_add_button);
    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
}

void SelectObjectDialogFuzzy::on_add_button() {
    if (edit->text().isEmpty()) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QString base = select_base_widget->get_base();

    const QString filter = [&]() {
        const QString entered_name = edit->text();

        const QString name_filter = filter_OR({
            filter_CONDITION(Condition_StartsWith, ATTRIBUTE_NAME, entered_name),
            filter_CONDITION(Condition_StartsWith, ATTRIBUTE_CN, entered_name),
            filter_CONDITION(Condition_StartsWith, ATTRIBUTE_SAMACCOUNT_NAME, entered_name),
            filter_CONDITION(Condition_StartsWith, ATTRIBUTE_USER_PRINCIPAL_NAME, entered_name),
        });

        const QString classes_filter = select_classes->get_filter();

        const QString out = filter_AND({
            name_filter,
            classes_filter,
        });

        return out;
    }();

    const QHash<QString, AdObject> search_results = ad.search(base, SearchScope_All, filter, console_object_search_attributes());

    if (search_results.size() == 1) {
        // Add to list
        const QList<QStandardItem *> row = make_item_row(g_adconfig->get_columns().count());

        const AdObject object = search_results.values()[0];
        console_object_load(row, object);

        model->appendRow(row);

        edit->clear();
    } else if (search_results.size() > 1) {
        // Open dialog where you can select one of the matches
        // TODO: probably make a separate file, decent sized dialog
        auto dialog = new SelectFuzzyMatchDialog(search_results, this);
        connect(
            dialog, &QDialog::accepted,
            [=]() {
                const QList<QString> selected_list = dialog->get_selected();
                // TODO: duplicating section above
                for (const QString &dn : selected_list) {
                    const AdObject object = search_results[dn];

                    const QList<QStandardItem *> row = make_item_row(g_adconfig->get_columns().count());
                    console_object_load(row, object);
                    model->appendRow(row);
                }

                edit->clear();
            });

        dialog->open();
    } else if (search_results.size() == 0) {
        // Warn about failing to find any matches
        QMessageBox::warning(this, tr("Error"), tr("Failed to find any matches."));
    }
}

SelectFuzzyMatchDialog::SelectFuzzyMatchDialog(const QHash<QString, AdObject> &search_results, QWidget *parent)
: QDialog() {
    setAttribute(Qt::WA_DeleteOnClose);

    auto label = new QLabel(tr("There are multiple matches. Select one or more to add to the list."));

    auto model = new QStandardItemModel(this);

    const QList<QString> header_labels = console_object_header_labels();
    model->setHorizontalHeaderLabels(header_labels);

    for (const AdObject &object : search_results) {
        const QList<QStandardItem *> row = make_item_row(g_adconfig->get_columns().count());

        console_object_load(row, object);

        model->appendRow(row);
    }

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->sortByColumn(0, Qt::AscendingOrder);

    view->setModel(model);

    auto button_box = new QDialogButtonBox();
    button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(label);
    layout->addWidget(view);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
}

QList<QString> SelectFuzzyMatchDialog::get_selected() const {
    QList<QString> out;

    const QList<QModelIndex> selected_indexes = view->selectionModel()->selectedRows();

    for (const QModelIndex &index : selected_indexes) {
        const QString dn = index.data(ObjectRole_DN).toString();
        out.append(dn);
    }

    return out;
}
