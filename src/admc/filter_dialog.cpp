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

#include "filter_dialog.h"

#include "adldap.h"
#include "globals.h"
#include "filter_widget/filter_widget.h"
#include "filter_custom_dialog.h"
#include "filter_classes_widget.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QCheckBox>
#include <QDebug>

// TODO: implement canceling. Need to be able to load/unload
// filter widget state though. For example, one way to
// implement would be to save old state on open, then reload
// it when cancel is pressed.

FilterDialog::FilterDialog(QWidget *parent)
: QDialog(parent)
{   
    setWindowTitle(tr("Filter contents"));
    resize(400, 400);

    // = all classes - container classes
    const QList<QString> noncontainer_classes =
    []() {
        QList<QString> out = filter_classes;

        const QList<QString> container_classes = g_adconfig->get_filter_containers();
        for (const QString &container_class : container_classes) {
            out.removeAll(container_class);
        }

        return out;
    }();

    filter_widget = new FilterWidget(noncontainer_classes);

    custom_dialog = new FilterCustomDialog(this);

    all_button = new QRadioButton(tr("Show all"));
    classes_button = new QRadioButton(tr("Show only these types"));
    custom_button = new QRadioButton(tr("Create custom"));

    all_button->setChecked(true);

    custom_dialog_button = new QPushButton(tr("Custom"));

    filter_classes_widget = new FilterClassesWidget();

    auto buttonbox = new QDialogButtonBox();
    buttonbox->addButton(QDialogButtonBox::Ok);

    auto radio_buttons_frame = new QFrame();
    radio_buttons_frame->setFrameStyle(QFrame::Raised);
    radio_buttons_frame->setFrameShape(QFrame::Box);

    auto classes_row_layout = new QVBoxLayout();
    classes_row_layout->addWidget(classes_button);
    classes_row_layout->addWidget(filter_classes_widget);

    auto custom_row_layout = new QHBoxLayout();
    custom_row_layout->addWidget(custom_button);
    custom_row_layout->addWidget(custom_dialog_button);

    auto radio_layout = new QVBoxLayout();
    radio_buttons_frame->setLayout(radio_layout);
    radio_layout->addWidget(all_button);
    radio_layout->addLayout(classes_row_layout);
    radio_layout->addLayout(custom_row_layout);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(radio_buttons_frame);
    layout->addWidget(buttonbox);

    connect(
        buttonbox, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        custom_dialog_button, &QPushButton::clicked,
        custom_dialog, &QDialog::open);

    connect(
        custom_button, &QAbstractButton::toggled,
        this, &FilterDialog::on_custom_button);
    on_custom_button();

    connect(
        classes_button, &QAbstractButton::toggled,
        this, &FilterDialog::on_classes_button);
    on_classes_button();
}

QString FilterDialog::get_filter() const {
    if (all_button->isChecked()) {
        return "(objectClass=*)";
    } else if (classes_button->isChecked()) {
        return filter_classes_widget->get_filter();
    } else if (custom_button->isChecked()) {
        return custom_dialog->filter_widget->get_filter();
    }
    
    return QString();
}

void FilterDialog::on_custom_button() {
    const bool checked = custom_button->isChecked();

    custom_dialog_button->setEnabled(checked);
}

void FilterDialog::on_classes_button() {
    const bool checked = classes_button->isChecked();

    filter_classes_widget->setEnabled(checked);
}
