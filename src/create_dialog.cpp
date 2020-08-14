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

#include "create_dialog.h"
#include "ad_interface.h"

#include <QDialog>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QList>

CreateDialog::CreateDialog(const QString &parent_arg, CreateType type_arg, QWidget *parent_widget)
: QDialog(parent_widget)
{
    parent = parent_arg;
    type = type_arg;

    setAttribute(Qt::WA_DeleteOnClose);
    resize(600, 600);

    const QString type_name = create_type_to_string(type);
    const auto title_label = new QLabel(QString(tr("Create object - \"%1\"")).arg(type_name), this);

    const auto ok_button = new QPushButton(tr("OK"), this);
    connect(
        ok_button, &QAbstractButton::clicked,
        this, &QDialog::accept);

    const auto cancel_button = new QPushButton(tr("Cancel"), this);
    connect(
        cancel_button, &QAbstractButton::clicked,
        this, &QDialog::reject);

    const auto label_layout = new QVBoxLayout();
    const auto edit_layout = new QVBoxLayout();

    const auto attributes_layout = new QHBoxLayout();
    attributes_layout->insertLayout(-1, label_layout);
    attributes_layout->insertLayout(-1, edit_layout);

    const auto top_layout = new QGridLayout(this);
    top_layout->addWidget(title_label, 0, 0);
    top_layout->addLayout(attributes_layout, 1, 0);
    top_layout->addWidget(cancel_button, 2, 0, Qt::AlignLeft);
    top_layout->addWidget(ok_button, 2, 2, Qt::AlignRight);

    auto name_edit_label = new QLabel(tr("Name:"));
    name_edit = new QLineEdit(this);

    label_layout->addWidget(name_edit_label);
    edit_layout->addWidget(name_edit);

    connect(
        this, &QDialog::accepted,
        this, &CreateDialog::on_accepted);
}

void CreateDialog::on_accepted() {
    const QString name = name_edit->text();

    auto get_suffix =
    [](CreateType type_arg) {
        switch (type_arg) {
            case CreateType::User: return "CN";
            case CreateType::Computer: return "CN";
            case CreateType::OU: return "OU";
            case CreateType::Group: return "CN";
            case CreateType::COUNT: return "COUNT";
        }
        return "";
    };
    const QString suffix = get_suffix(type);

    auto get_classes =
    [](CreateType type_arg) {
        static const char *classes_user[] = {CLASS_USER, NULL};
        static const char *classes_group[] = {CLASS_GROUP, NULL};
        static const char *classes_ou[] = {CLASS_OU, NULL};
        static const char *classes_computer[] = {CLASS_TOP, CLASS_PERSON, CLASS_ORG_PERSON, CLASS_USER, CLASS_COMPUTER, NULL};

        switch (type_arg) {
            case User: return classes_user;
            case Computer: return classes_computer;
            case OU: return classes_ou;
            case Group: return classes_group;
            case COUNT: return classes_user;
        }
        return classes_user;
    };
    const char **classes = get_classes(type);
    
    const QString dn = suffix + "=" + name + "," + parent;

    AdInterface::instance()->object_add(dn, classes);
}
