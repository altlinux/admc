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

#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QPlainTextEdit>

SelectObjectDialogFuzzy::SelectObjectDialogFuzzy(const QList<QString> classes, QWidget *parent)
: QDialog(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Select dialog");

    resize(600, 500);

    select_classes = new SelectClassesWidget(classes);

    edit = new QPlainTextEdit();

    auto add_button = new QPushButton(tr("Add"));

    model = new QStandardItemModel(this);

    const QList<QString> header_labels = console_object_header_labels();
    model->setHorizontalHeaderLabels(header_labels);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->sortByColumn(0, Qt::AscendingOrder);

    view->setModel(model);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(select_classes);
    layout->addWidget(edit);
    layout->addWidget(add_button);
    layout->addWidget(view);

    connect(
        add_button, &QPushButton::clicked,
        this, &SelectObjectDialogFuzzy::on_add_button);
}

void SelectObjectDialogFuzzy::on_add_button() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QString filter = [&]() {
        const QString entered_name = edit->toPlainText();

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

    const QHash<QString, AdObject> search_results = ad.search(g_adconfig->domain_head(), SearchScope_All, filter, console_object_search_attributes());

    if (search_results.size() == 1) {
        // Add to list
        const QList<QStandardItem *> row = make_item_row(g_adconfig->get_columns().count());

        const AdObject object = search_results.values()[0];
        console_object_load(row, object);

        model->appendRow(row);
    } else if (search_results.size() > 1) {
        // Open dialog where you can select one of the matches
        // TODO: probably make a separate file, decent sized dialog
    } else if (search_results.size() == 0) {
        // Warn about failing to find any matches
        QMessageBox::warning(this, tr("Error"), tr("Failed to find any matches."));
    }
}
