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
#include "settings.h"
#include "confirmation_dialog.h"
#include "dn_column_proxy.h"
#include "utils.h"

#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QAction>

const QString CONTAINER_STR = "container";
const QString OU_STR = "organizationalUnit";

const QMap<ClassFilterType, QString> class_filter_display_text = {
    {ClassFilterType_All, "All"},
    {ClassFilterType_Containers, "Containers"},
    {ClassFilterType_OUs, "OU's"},
};

const QMap<ClassFilterType, QString> class_filter_string = {
    {ClassFilterType_All, ""},
    {ClassFilterType_Containers, CONTAINER_STR},
    {ClassFilterType_OUs, OU_STR},
};

MoveDialog::MoveDialog(QWidget *parent)
: QDialog(parent)
{
    setModal(true);
    resize(600, 600);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    target_label = new QLabel("TARGET");

    const auto filter_class_label = new QLabel("Class: ");
    filter_class_combo_box = new QComboBox(this);

    const auto filter_name_label = new QLabel("Name: ");
    filter_name_line_edit = new QLineEdit(this);

    model = new MoveDialogModel(this);

    proxy_name = new QSortFilterProxyModel(this);
    proxy_name->setFilterKeyColumn(MoveDialogModel::Column::Name);

    proxy_class = new QSortFilterProxyModel(this);
    proxy_class->setFilterKeyColumn(MoveDialogModel::Column::Class);

    const auto dn_column_proxy = new DnColumnProxy(MoveDialogModel::Column::DN, this);

    setup_model_chain(view, model, {proxy_name, proxy_class, dn_column_proxy});

    const auto layout = new QGridLayout(this);
    layout->addWidget(target_label, 0, 0);
    layout->addWidget(filter_class_label, 1, 0);
    layout->addWidget(filter_class_combo_box, 1, 1, 1, 2);
    layout->addWidget(filter_name_label, 2, 0);
    layout->addWidget(filter_name_line_edit, 2, 1, 1, 2);
    layout->addWidget(view, 3, 0, 1, 3);

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
    const QString target_label_text = QString("Moving \"%1\"").arg(target_dn);
    target_label->setText(target_label_text);

    filter_name_line_edit->setText("");
    on_filter_name_changed("");

    // Select classes that this entry can be moved to
    QList<ClassFilterType> classes;
    const bool is_container = AD()->is_container(dn);
    if (is_container) {
        classes = {ClassFilterType_Containers};
    } else {
        classes = {ClassFilterType_Containers, ClassFilterType_OUs};
    }

    filter_class_combo_box->clear();
    QList<ClassFilterType> combo_classes = classes;
    combo_classes.insert(0, ClassFilterType_All);
    for (auto c : combo_classes) {
        const QString string = class_filter_display_text[c];

        filter_class_combo_box->addItem(string, c);
    }

    // Load model
    model->load(dn, classes);

    for (int col = 0; col < MoveDialogModel::Column::COUNT; col++) {
        view->resizeColumnToContents(col);
    }

    exec();
}

void MoveDialog::on_filter_name_changed(const QString &text) {
    proxy_name->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive, QRegExp::FixedString));
}

void MoveDialog::on_filter_class_changed(int index) {
    QVariant item_data = filter_class_combo_box->itemData(index);
    const ClassFilterType type = item_data.value<ClassFilterType>();
    const QString regexp = class_filter_string[type];

    proxy_class->setFilterRegExp(QRegExp(regexp, Qt::CaseInsensitive, QRegExp::FixedString));
}

void MoveDialog::on_double_clicked(const QModelIndex &index) {
    const QModelIndex dn_index = index.siblingAtColumn(MoveDialogModel::Column::DN);
    const QString dn = dn_index.data().toString();

    const QString confirm_text = QString("Move \"%1\" to \"%2\"?").arg(target_dn, dn);
    const bool confirmed = confirmation_dialog(confirm_text, this);
    if (confirmed) {
        AD()->move(target_dn, dn);
        done(QDialog::Accepted);
    }
}

MoveDialogModel::MoveDialogModel(QObject *parent)
: QStandardItemModel(0, Column::COUNT, parent)
{
    setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    setHorizontalHeaderItem(Column::Class, new QStandardItem("Class"));
}

void MoveDialogModel::load(const QString &dn, QList<ClassFilterType> classes) {
    removeRows(0, rowCount());

    const QAction *const toggle_advanced_view = SETTINGS()->toggle_advanced_view;
    const bool advanced_view_is_off = !toggle_advanced_view->isChecked();

    for (auto c : classes) {
        const QString class_string = class_filter_string[c];

        QString filter = filter_EQUALS("objectClass", class_string);
        if (advanced_view_is_off) {
            // Exclude advanced view entries
            const QString advanced_true = filter_EQUALS("showInAdvancedViewOnly", "TRUE");
            const QString NOT_advanced_true = filter_NOT(advanced_true);
            filter = filter_AND(filter, NOT_advanced_true);
        }

        const QList<QString> entries = AD()->search(filter);

        for (auto e_dn : entries) {
            auto row = QList<QStandardItem *>();
            for (int i = 0; i < Column::COUNT; i++) {
                row.push_back(new QStandardItem());
            }

            const QString name = extract_name_from_dn(e_dn);

            row[Column::Name]->setText(name);
            row[Column::Class]->setText(class_string);
            row[Column::DN]->setText(e_dn);

            appendRow(row);
        }
    }
}
