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

#include "find_dialog.h"
#include "ad_interface.h"
#include "ad_utils.h"
#include "ad_config.h"
#include "settings.h"
#include "utils.h"
#include "filter.h"
#include "filter_widget/filter_widget.h"
#include "find_results.h"
#include "select_container_dialog.h"
#include "object_menu.h"

#include <QString>
#include <QList>
#include <QTreeView>
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>
#include <QDebug>
#include <QCheckBox>
#include <QMenuBar>
#include <QDialogButtonBox>

FindDialog::FindDialog(const FindDialogType type_arg, const QList<QString> classes, const QString &default_search_base, QWidget *parent)
: QDialog(parent)
{
    type = type_arg;

    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("Find objects"));

    // TODO: technically, entire directory does NOT equal to the domain. In cases where we're browsing multiple domains at the same time (or maybe some other situations as well), we'd need "Entire directory" AND all of domains. Currently search base is set to domain anyway, so would need to start from reworking that.

    search_base_combo = new QComboBox();
    search_base_combo->addItem(tr("Entire directory"), AD()->domain_head());
    const QString default_search_base_name = dn_get_name(default_search_base);
    search_base_combo->addItem(default_search_base_name, default_search_base);
    search_base_combo->setCurrentIndex(1);

    auto custom_search_base_button = new QPushButton(tr("Browse"));
    custom_search_base_button->setAutoDefault(false);

    filter_widget = new FilterWidget(classes);

    auto quick_find_check = new QCheckBox(tr("Quick find"));

    auto find_button = new QPushButton(tr("Find"));
    find_button->setAutoDefault(false);

    auto stop_button = new QPushButton(tr("Stop"));
    stop_button->setAutoDefault(false);

    find_results = new FindResults(type);

    auto filter_widget_frame = new QFrame();
    filter_widget_frame->setFrameStyle(QFrame::Raised);
    filter_widget_frame->setFrameShape(QFrame::Box);

    auto menubar = new QMenuBar();

    auto action_menu = new ObjectMenu(this);
    action_menu->setTitle(tr("&Action"));
    menubar->addMenu(action_menu);

    auto select_button_box = new QDialogButtonBox();
    select_button_box->addButton(QDialogButtonBox::Ok);
    select_button_box->addButton(QDialogButtonBox::Cancel);

    {
        auto search_base_layout = new QHBoxLayout();
        search_base_layout->addWidget(search_base_combo);
        search_base_layout->addWidget(custom_search_base_button);

        auto search_base_row = new QFormLayout();
        search_base_row->addRow(tr("Search in:"), search_base_layout);

        auto buttons_layout = new QHBoxLayout();
        buttons_layout->addWidget(find_button);
        buttons_layout->addWidget(stop_button);
        buttons_layout->addStretch(1);

        auto layout = new QVBoxLayout();
        filter_widget_frame->setLayout(layout);
        layout->addLayout(search_base_row);
        layout->addWidget(filter_widget);
        layout->addWidget(quick_find_check);
        layout->addLayout(buttons_layout);
    }

    {
        auto h_layout = new QHBoxLayout();
        h_layout->addWidget(filter_widget_frame);
        h_layout->addWidget(find_results);

        auto layout = new QVBoxLayout();
        setLayout(layout);
        layout->setMenuBar(menubar);
        layout->addLayout(h_layout);
        layout->addWidget(select_button_box);
    }

    // Keep filter widget compact, so that when user
    // expands find dialog horizontally, filter widget will
    // keep it's size, find results will get expanded
    filter_widget_frame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    find_results->setMinimumSize(500, 0);

    // Change some things based on type
    select_button_box->setVisible(type == FindDialogType_Select);
    menubar->setVisible(type == FindDialogType_Normal);

    connect(
        custom_search_base_button, &QAbstractButton::clicked,
        this, &FindDialog::select_custom_search_base);
    connect(
        find_button, &QPushButton::clicked,
        this, &FindDialog::find);
    connect(
        stop_button, &QPushButton::clicked,
        AD(), &AdInterface::stop_search);
    connect(
        filter_widget, &FilterWidget::return_pressed,
        this, &FindDialog::find);
    connect(
        filter_widget, &FilterWidget::changed,
        this, &FindDialog::on_filter_changed);

    connect(
        select_button_box, &QDialogButtonBox::accepted,
        this, &FindDialog::accept);
    connect(
        select_button_box, &QDialogButtonBox::rejected,
        this, &FindDialog::reject);

    SETTINGS()->connect_checkbox_to_bool_setting(quick_find_check, BoolSetting_QuickFind);

    action_menu->setup_as_menubar_menu(find_results->view, ADCONFIG()->get_column_index(ATTRIBUTE_DN));
}

void FindDialog::select_custom_search_base() {
    auto dialog = new SelectContainerDialog(parentWidget());
    dialog->setWindowTitle(tr("Select custom search base"));

    connect(
        dialog, &SelectContainerDialog::accepted,
        [this, dialog]() {
            const QString selected = dialog->get_selected();
            const QString name = dn_get_name(selected);

            search_base_combo->addItem(name, selected);

            // Select newly added search base in combobox
            const int new_base_index = search_base_combo->count() - 1;
            search_base_combo->setCurrentIndex(new_base_index);
        });

    dialog->open();
}

void FindDialog::on_filter_changed() {
    const bool quick_find = SETTINGS()->get_bool(BoolSetting_QuickFind);

    if (quick_find) {
        find();
    }
}

void FindDialog::find() {
    show_busy_indicator();

    const QString filter = filter_widget->get_filter();
    const QString search_base =
    [this]() {
        const int index = search_base_combo->currentIndex();
        const QVariant item_data = search_base_combo->itemData(index);

        return item_data.toString();
    }();

    find_results->load(filter, search_base);

    hide_busy_indicator();
}

QList<QList<QStandardItem *>> FindDialog::get_selected_rows() const {
    return find_results->get_selected_rows();
}
