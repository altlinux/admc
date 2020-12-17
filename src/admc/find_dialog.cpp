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
#include "containers_widget.h"
#include "utils.h"
#include "filter.h"
#include "filter_widget/filter_widget.h"
#include "find_results.h"
#include "select_dialog.h"

#include <QString>
#include <QList>
#include <QLineEdit>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QPushButton>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QDebug>
#include <QCheckBox>

FindDialog::FindDialog(QWidget *parent)
: QDialog(parent)
{
    setWindowTitle(tr("Find objects"));

    // TODO: technically, entire directory does NOT equal to the domain. In cases where we're browsing multiple domains at the same time (or maybe some other situations as well), we'd need "Entire directory" AND all of domains. Currently search base is set to domain anyway, so would need to start from reworking that.

    search_base_combo = new QComboBox();
    search_base_combo->addItem(tr("Entire directory"), AD()->domain_head());

    const QString users_dn = "CN=Users," + AD()->domain_head();
    search_base_combo->addItem("Users", users_dn);

    auto custom_search_base_button = new QPushButton(tr("Browse"));
    custom_search_base_button->setAutoDefault(false);

    filter_widget = new FilterWidget();

    auto quick_find_check = new QCheckBox(tr("Quick find"));

    auto find_button = new QPushButton(tr("Find"));
    find_button->setAutoDefault(false);

    find_results = new FindResults();

    auto filter_widget_frame = new QFrame();
    filter_widget_frame->setFrameStyle(QFrame::Raised);
    filter_widget_frame->setFrameShape(QFrame::Box);
    
    {
        auto search_base_layout = new QHBoxLayout();
        search_base_layout->addWidget(search_base_combo);
        search_base_layout->addWidget(custom_search_base_button);

        auto search_base_row = new QFormLayout();
        search_base_row->addRow(tr("Search in:"), search_base_layout);

        auto layout = new QVBoxLayout();
        filter_widget_frame->setLayout(layout);
        layout->addLayout(search_base_row);
        layout->addWidget(filter_widget);
        layout->addWidget(quick_find_check);
        layout->addWidget(find_button);
    }

    {
        auto layout = new QHBoxLayout();
        setLayout(layout);
        layout->addWidget(filter_widget_frame);
        layout->addWidget(find_results);
    }

    // Keep filter widget compact, so that when user
    // expands find dialog horizontally, filter widget will
    // keep it's size, find results will get expanded
    filter_widget_frame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    find_results->setMinimumSize(500, 0);

    connect(
        custom_search_base_button, &QAbstractButton::clicked,
        this, &FindDialog::select_custom_search_base);
    connect(
        find_button, &QPushButton::clicked,
        this, &FindDialog::find);
    connect(
        filter_widget, &FilterWidget::return_pressed,
        this, &FindDialog::find);
    connect(
        filter_widget, &FilterWidget::changed,
        this, &FindDialog::on_filter_changed);

    SETTINGS()->connect_checkbox_to_bool_setting(quick_find_check, BoolSetting_QuickFind);
}

void FindDialog::select_custom_search_base() {
    // TODO: maybe need some other classes?
    const QString title = QString(tr("Select search base"));
    const QList<QString> selecteds = SelectDialog::open({CLASS_CONTAINER, CLASS_OU}, SelectDialogMultiSelection_Yes, title, this);

    if (!selecteds.isEmpty()) {
        const QString selected = selecteds[0];
        const QString name = dn_get_name(selected);

        search_base_combo->addItem(name, selected);

        // Select newly added search base in combobox
        const int new_base_index = search_base_combo->count() - 1;
        search_base_combo->setCurrentIndex(new_base_index);
    }
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
