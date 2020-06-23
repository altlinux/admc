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

#include "move_dialog.h"
#include "ad_interface.h"

#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QComboBox>

enum Column {
    Column_Name,
    Column_Class,
    Column_DN,
    Column_COUNT
};

enum ClassFilterType {
    ClassFilterType_All,
    ClassFilterType_Containers,
    ClassFilterType_OUs,
    ClassFilterType_COUNT
};

const QString CONTAINER_STR = "container";
const QString OU_STR = "organizationalUnit";

const QMap<ClassFilterType, QString> class_filter_string = {
    {ClassFilterType_All, "All"},
    {ClassFilterType_Containers, "Containers"},
    {ClassFilterType_OUs, "OU's"},
};

const QMap<ClassFilterType, QString> class_filter_regexp = {
    {ClassFilterType_All, ""},
    {ClassFilterType_Containers, CONTAINER_STR},
    {ClassFilterType_OUs, OU_STR},
};

MoveDialog::MoveDialog(QWidget *parent)
: QDialog(parent)
{
    setModal(true);

    //
    // Objects
    //
    const auto filter_class_label = new QLabel("Class: ");
    const auto filter_class_combo_box = new QComboBox(this);
    for (int i = 0; i < ClassFilterType_COUNT; i++) {
        const ClassFilterType type = static_cast<ClassFilterType>(i);
        const QString string = class_filter_string[type];

        filter_class_combo_box->addItem(string);
    }

    const auto filter_name_label = new QLabel("Name: ");
    filter_name_line_edit = new QLineEdit(this);

    model = new QStandardItemModel(0, Column_COUNT, this);
    model->setHorizontalHeaderItem(Column_Name, new QStandardItem("Name"));
    model->setHorizontalHeaderItem(Column_Class, new QStandardItem("Class"));

    proxy_name = new QSortFilterProxyModel(this);
    proxy_name->setSourceModel(model);
    proxy_name->setFilterKeyColumn(Column_Name);

    proxy_class = new QSortFilterProxyModel(this);
    proxy_class->setSourceModel(proxy_name);
    proxy_class->setFilterKeyColumn(Column_Class);

    view = new QTreeView(this);
    view->setModel(proxy_class);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //
    // Layout
    //
    resize(600, 600);

    const auto layout = new QGridLayout(this);
    layout->addWidget(filter_class_label, 0, 0);
    layout->addWidget(filter_class_combo_box, 0, 1, 1, 2);
    layout->addWidget(filter_name_label, 1, 0);
    layout->addWidget(filter_name_line_edit, 1, 1, 1, 2);
    layout->addWidget(view, 2, 0, 1, 3);

    connect(
        filter_name_line_edit, &QLineEdit::textChanged,
        this, &MoveDialog::on_filter_name_changed);
    connect(filter_class_combo_box, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MoveDialog::on_filter_class_changed);
    connect(
        view, &QAbstractItemView::doubleClicked,
        this, &MoveDialog::on_double_clicked);
}

void MoveDialog::open_for_entry(const QString &dn) {
    target_dn = dn;

    filter_name_line_edit->setText("");
    on_filter_name_changed("");

    // Load model
    model->removeRows(0, model->rowCount());

    const QList<QString> containers = AD()->search("objectClass", CONTAINER_STR);
    const QList<QString> OUs = AD()->search("objectClass", OU_STR);

    auto add_list =
    [this]
    (const QList<QString> &list, const QString &class_str) {
        for (auto e_dn : list) {
            // TODO: make entryproxymodel into AdvancedFilter
            // that accepts any model and takes dn_column as ctor arg
            // to get the dn
            // and then use that to filter here properly
            const bool advanced_view_only = AD()->get_attribute(e_dn, "showInAdvancedViewOnly") == "TRUE";
            if (advanced_view_only) {
                continue;
            }

            auto row = QList<QStandardItem *>();
            for (int i = 0; i < Column_COUNT; i++) {
                row.push_back(new QStandardItem());
            }

            const QString name = AD()->get_attribute(e_dn, "name");

            row[Column_Name]->setText(name);
            row[Column_Class]->setText(class_str);
            row[Column_DN]->setText(e_dn);

            model->appendRow(row);
        }
    };

    add_list(containers, CONTAINER_STR);
    add_list(OUs, OU_STR);

    for (int col = 0; col < Column_COUNT; col++) {
        view->resizeColumnToContents(col);
    }

    open();
}

void MoveDialog::on_filter_name_changed(const QString &text) {
    proxy_name->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive, QRegExp::FixedString));
}

void MoveDialog::on_filter_class_changed(int index) {
    const ClassFilterType type = static_cast<ClassFilterType>(index);
    const QString regexp = class_filter_regexp[type];

    proxy_class->setFilterRegExp(QRegExp(regexp, Qt::CaseInsensitive, QRegExp::FixedString));
}

void MoveDialog::on_double_clicked(const QModelIndex &index) {
    const QModelIndex dn_index = index.siblingAtColumn(Column_DN);
    const QString dn = dn_index.data().toString();

    AD()->move(target_dn, dn);
}
