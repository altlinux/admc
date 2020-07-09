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
#include <QGridLayout>
#include <QLabel>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QAction>
#include <QPushButton>
#include <QItemSelectionModel>

enum MoveDialogColumn {
    MoveDialogColumn_Name,
    MoveDialogColumn_Class,
    MoveDialogColumn_DN,
    MoveDialogColumn_COUNT
};

const QMap<ClassFilter, QString> class_filter_display_text = {
    {ClassFilter_All, "All"},
    {ClassFilter_Containers, "Containers"},
    {ClassFilter_OUs, "OU's"},
    {ClassFilter_Groups, "Groups"},
};

const QMap<ClassFilter, QString> class_filter_string = {
    {ClassFilter_All, ""},
    {ClassFilter_Containers, "container"},
    {ClassFilter_OUs, "organizationalUnit"},
    {ClassFilter_Groups, "group"},
};

MoveDialog::MoveDialog(QWidget *parent)
: QDialog(parent)
{
    resize(600, 600);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    target_label = new QLabel("TARGET");

    filter_class_label = new QLabel("Class: ");
    filter_class_combo_box = new QComboBox(this);

    const auto filter_name_label = new QLabel("Name: ");
    filter_name_line_edit = new QLineEdit(this);

    const auto select_button = new QPushButton("Select", this);
    const auto cancel_button = new QPushButton("Cancel", this);

    const auto layout = new QGridLayout(this);
    layout->addWidget(target_label, 0, 0);
    layout->addWidget(filter_class_label, 1, 0, Qt::AlignRight);
    layout->addWidget(filter_class_combo_box, 1, 1, 1, 2);
    layout->addWidget(filter_name_label, 2, 0, Qt::AlignRight);
    layout->addWidget(filter_name_line_edit, 2, 1, 1, 2);
    layout->addWidget(view, 3, 0, 1, 3);
    layout->addWidget(cancel_button, 4, 0, Qt::AlignLeft);
    layout->addWidget(select_button, 4, 2, Qt::AlignRight);

    model = new MoveDialogModel(this);

    proxy_name = new QSortFilterProxyModel(this);
    proxy_name->setFilterKeyColumn(MoveDialogColumn_Name);

    proxy_class = new QSortFilterProxyModel(this);
    proxy_class->setFilterKeyColumn(MoveDialogColumn_Class);

    const auto dn_column_proxy = new DnColumnProxy(MoveDialogColumn_DN, this);

    setup_model_chain(view, model, {proxy_name, proxy_class, dn_column_proxy});

    connect(
        view, &QAbstractItemView::doubleClicked,
        this, &MoveDialog::on_double_clicked);
    connect(
        filter_name_line_edit, &QLineEdit::textChanged,
        this, &MoveDialog::on_filter_name_changed);
    connect(filter_class_combo_box, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MoveDialog::on_filter_class_changed);
    connect(
        select_button, &QAbstractButton::clicked,
        this, &MoveDialog::on_select_button);
    connect(
        cancel_button, &QAbstractButton::clicked,
        this, &MoveDialog::on_cancel_button);
    connect(
        this, &QDialog::finished,
        model, &MoveDialogModel::on_dialog_finished);
}

void MoveDialog::open_for_entry(const QString &dn, MoveDialogType type_arg) {
    target_dn = dn;
    type = type_arg;

    filter_name_line_edit->setText("");
    on_filter_name_changed("");

    QString target_label_text;
    switch (type) {
        case MoveDialogType_Move: {
            target_label_text = QString("Moving \"%1\"").arg(target_dn);
            break;
        }
        case MoveDialogType_AddToGroup: {
            target_label_text = QString("Adding \"%1\" to group").arg(target_dn);
            break;
        }
    }
    target_label->setText(target_label_text);

    // Select classes that this entry can be moved to
    // TODO: cover all cases
    QList<ClassFilter> classes;
    switch (type) {
        case MoveDialogType_Move: {
            const bool is_container = AD()->is_container(dn);
            if (is_container) {
                classes = {ClassFilter_Containers};
            } else {
                classes = {ClassFilter_Containers, ClassFilter_OUs};
            }
            break;
        }
        case MoveDialogType_AddToGroup: {
            classes = {ClassFilter_Groups};
            break;
        }
    }

    // Fill class combo box with possible classes and "All" option
    filter_class_combo_box->clear();
    QList<ClassFilter> combo_classes = {ClassFilter_All};
    combo_classes += classes;
    for (auto c : combo_classes) {
        const QString string = class_filter_display_text[c];

        filter_class_combo_box->addItem(string, c);
    }

    // Show or hide class-related elements depending on type
    switch (type) {
        case MoveDialogType_Move: {
            filter_class_combo_box->show();
            filter_class_label->show();
            view->setColumnHidden(MoveDialogColumn_Class, false);

            break;
        }
        case MoveDialogType_AddToGroup: {
            filter_class_combo_box->hide();
            filter_class_label->hide();
            view->setColumnHidden(MoveDialogColumn_Class, true);

            break;
        }
    }

    model->load(dn, classes);

    for (int col = 0; col < view->model()->columnCount(); col++) {
        view->resizeColumnToContents(col);
    }

    open();
}

void MoveDialog::on_filter_name_changed(const QString &text) {
    proxy_name->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive, QRegExp::FixedString));
}

void MoveDialog::on_filter_class_changed(int index) {
    const QVariant item_data = filter_class_combo_box->itemData(index);
    const ClassFilter class_filter = item_data.value<ClassFilter>();
    const QString regexp = class_filter_string[class_filter];

    proxy_class->setFilterRegExp(QRegExp(regexp, Qt::CaseInsensitive, QRegExp::FixedString));
}

void MoveDialog::complete(const QString &move_dn) {
    const QString confirm_text = QString("Move \"%1\" to \"%2\"?").arg(target_dn, move_dn);

    const bool confirmed = confirmation_dialog(confirm_text, this);
    if (confirmed) {
        switch (type) {
            case MoveDialogType_Move: {
                AD()->move(target_dn, move_dn);
                break;
            }
            case MoveDialogType_AddToGroup: {
                AD()->add_user_to_group(move_dn, target_dn);
                break;
            }
        }
        done(QDialog::Accepted);
    }
}

void MoveDialog::on_select_button(bool) {
    const QItemSelectionModel *selection_model = view->selectionModel();
    if (!selection_model->hasSelection()) {
        return;
    }
    const QModelIndex selected_index = selection_model->currentIndex();
    const QString move_dn = get_dn_from_index(selected_index, MoveDialogColumn_DN);
    
    complete(move_dn);
}

void MoveDialog::on_double_clicked(const QModelIndex &index) {
    const QString move_dn = get_dn_from_index(index, MoveDialogColumn_DN);

    complete(move_dn);
}

void MoveDialog::on_cancel_button(bool) {
    done(QDialog::Rejected);
}

MoveDialogModel::MoveDialogModel(QObject *parent)
: QStandardItemModel(0, MoveDialogColumn_COUNT, parent)
{
    setHorizontalHeaderItem(MoveDialogColumn_Name, new QStandardItem("Name"));
    setHorizontalHeaderItem(MoveDialogColumn_Class, new QStandardItem("Class"));
    setHorizontalHeaderItem(MoveDialogColumn_DN, new QStandardItem("DN"));
}

void MoveDialogModel::load(const QString &dn, QList<ClassFilter> classes) {
    for (auto c : classes) {
        const QString class_string = class_filter_string[c];

        QString filter = filter_EQUALS("objectClass", class_string);

        // Filter out advanced entries if needed
        const QAction *advanced_view_action = SETTINGS()->checkable(SettingsCheckable_AdvancedView);
        const bool advanced_view_is_off = !advanced_view_action->isChecked();

        if (advanced_view_is_off) {
            const QString is_advanced = filter_EQUALS("showInAdvancedViewOnly", "TRUE");
            const QString NOT_is_advanced = filter_NOT(is_advanced);
            
            filter = filter_AND(filter, NOT_is_advanced);
        }

        const QList<QString> entries = AD()->search(filter);

        for (auto e_dn : entries) {
            auto row = QList<QStandardItem *>();
            for (int i = 0; i < MoveDialogColumn_COUNT; i++) {
                row.push_back(new QStandardItem());
            }

            const QString name = extract_name_from_dn(e_dn);

            row[MoveDialogColumn_Name]->setText(name);
            row[MoveDialogColumn_Class]->setText(class_string);
            row[MoveDialogColumn_DN]->setText(e_dn);

            appendRow(row);
        }
    }
}

void MoveDialogModel::on_dialog_finished(int) {
    removeRows(0, rowCount());
}
