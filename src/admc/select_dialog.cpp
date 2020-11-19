/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "select_dialog.h"
#include "ad_interface.h"
#include "settings.h"
#include "containers_widget.h"
#include "ad_config.h"
#include "utils.h"
#include "filter.h"
#include "config.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QPushButton>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <algorithm>

enum SelectDialogColumn {
    SelectDialogColumn_Name,
    SelectDialogColumn_Parent,
    SelectDialogColumn_Class,
    SelectDialogColumn_DN,
    SelectDialogColumn_COUNT
};

QList<QString> SelectDialog::open(QList<QString> classes, SelectDialogMultiSelection multi_selection) {
    SelectDialog dialog(classes, multi_selection);
    dialog.exec();

    return dialog.selected_objects;
}

SelectDialog::SelectDialog(QList<QString> classes, SelectDialogMultiSelection multi_selection)
: QDialog()
{
    setWindowTitle(QString(tr("Select object - %1")).arg(ADMC_APPLICATION_NAME));

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSortingEnabled(true);

    if (multi_selection == SelectDialogMultiSelection_Yes) {
        view->setSelectionMode(QAbstractItemView::ContiguousSelection);
    }

    auto filter_class_label = new QLabel(tr("Class: "), this);
    auto filter_class_combo = new QComboBox(this);

    const auto filter_name_label = new QLabel(tr("Name: "), this);
    auto filter_name_edit = new QLineEdit(this);

    const auto select_button = new QPushButton(tr("Select"), this);
    const auto cancel_button = new QPushButton(tr("Cancel"), this);

    const auto layout = new QGridLayout(this);
    layout->addWidget(filter_class_label, 1, 0, Qt::AlignRight);
    layout->addWidget(filter_class_combo, 1, 1, 1, 2);
    layout->addWidget(filter_name_label, 2, 0, Qt::AlignRight);
    layout->addWidget(filter_name_edit, 2, 1, 1, 2);
    layout->addWidget(view, 3, 0, 1, 3);
    layout->addWidget(cancel_button, 4, 0, Qt::AlignLeft);
    layout->addWidget(select_button, 4, 2, Qt::AlignRight);

    auto proxy_name = new QSortFilterProxyModel(this);
    proxy_name->setFilterKeyColumn(SelectDialogColumn_Name);
    proxy_name->setRecursiveFilteringEnabled(true);
    
    auto proxy_class = new QSortFilterProxyModel(this);
    proxy_class->setFilterKeyColumn(SelectDialogColumn_Class);
    proxy_class->setRecursiveFilteringEnabled(true);

    // Load model
    auto model = new QStandardItemModel(0, SelectDialogColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model, {
        {SelectDialogColumn_Name, tr("Name")},
        {SelectDialogColumn_Parent, tr("Parent")},
        {SelectDialogColumn_Class, tr("Class")},
        {SelectDialogColumn_DN, tr("DN")}
    });

    const QString filter =
    [classes]() {
        QList<QString> subfilters;
        for (int i = 0; i < classes.size(); i++) {
            const QString object_class = classes[i];
            const QString equals_to_class = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, object_class);

            subfilters.append(equals_to_class);
        }

        return filter_OR(subfilters);
    }();

    const QList<QString> attributes = {ATTRIBUTE_OBJECT_CLASS};
    const QHash<QString, AdObject> search_results = AD()->search(filter, attributes, SearchScope_All);

    for (const AdObject &object : search_results.values()) {
        // TODO: get name from attribute
        const QString dn = object.get_dn();
        const QString name = dn_get_name(dn);
        const QString parent = dn_get_parent_canonical(dn);

        const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);

        // NOTE: search returns objects if ANY of their classes are equal to given ones, we only want to get objects by their most inherited class(last), so filter out others
        if (!classes.contains(object_class)) {
            continue;
        }

        const QList<QStandardItem *> row = make_item_row(SelectDialogColumn_COUNT);
        row[SelectDialogColumn_Name]->setText(name);
        row[SelectDialogColumn_Parent]->setText(parent);
        row[SelectDialogColumn_Class]->setText(object_class);
        row[SelectDialogColumn_DN]->setText(dn);

        model->appendRow(row);
    }

    model->sort(SelectDialogColumn_Name);

    proxy_name->setSourceModel(model);
    proxy_class->setSourceModel(proxy_name);
    view->setModel(proxy_class);

    setup_column_toggle_menu(view, model, {SelectDialogColumn_Name, SelectDialogColumn_Parent});

    // Fill class combo box with possible classes
    filter_class_combo->clear();
    for (auto object_class : classes) {
        auto class_name = ADCONFIG()->get_class_display_name(object_class);

        filter_class_combo->addItem(class_name, object_class);
    }
    filter_class_combo->addItem(tr("All"), "");
    filter_class_combo->setCurrentIndex(filter_class_combo->count() - 1);

    // Disable/hide class-related elements if selecting only from one class
    if (classes.size() == 1) {
        filter_class_combo->setEnabled(false);
        view->setColumnHidden(SelectDialogColumn_Class, true);
    }

    for (int col = 0; col < view->model()->columnCount(); col++) {
        view->resizeColumnToContents(col);
    }

    // Disable select button when there's no selection and enable when there is
    const QItemSelectionModel *selection_model = view->selectionModel();
    connect(selection_model, &QItemSelectionModel::selectionChanged,
        [selection_model, select_button]() {
            const bool enable_select_button = selection_model->hasSelection();
            select_button->setEnabled(enable_select_button);
        });

    // Update proxy name when filter name changes
    connect(
        filter_name_edit, &QLineEdit::textChanged,
        [proxy_name](const QString &text) {
            proxy_name->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive, QRegExp::FixedString));
        });
    filter_name_edit->setText("");

    // Update proxy class when filter class changes
    connect(filter_class_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [filter_class_combo, proxy_class](const int index) {
            const QVariant item_data = filter_class_combo->itemData(index);
            const QString object_class = item_data.toString();

            proxy_class->setFilterRegExp(QRegExp(object_class, Qt::CaseInsensitive, QRegExp::FixedString));
        });

    connect(
        select_button, &QAbstractButton::clicked,
        this, &QDialog::accept);
    connect(
        cancel_button, &QAbstractButton::clicked,
        this, &QDialog::reject);
}

void SelectDialog::accept() {
    const QItemSelectionModel *selection_model = view->selectionModel();
    const QList<QModelIndex> selected_indexes = selection_model->selectedIndexes();

    selected_objects.clear();
    for (auto index : selected_indexes) {
        const QString dn = get_dn_from_index(index, SelectDialogColumn_DN);

        if (!selected_objects.contains(dn)) {
            selected_objects.append(dn);
        }
    }

    QDialog::accept();
}
